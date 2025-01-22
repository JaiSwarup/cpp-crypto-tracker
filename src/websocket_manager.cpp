#include "websocket_manager.hpp"
#include <nlohmann/json.hpp>
#include "performance_tracker.hpp"
#include <iostream>

using json = nlohmann::json;

WebSocketServer::WebSocketServer(DeribitClient& deribit_client)
    : m_running(false), m_deribit_client(deribit_client) {
    
    logger = Logger();
    m_server.clear_access_channels(websocketpp::log::alevel::all);
    m_server.init_asio();

    m_server.set_open_handler([this](connection_hdl hdl) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_connections.insert(hdl);
        logger.log(Logger::LogLevel::INFO, "Client connected");
    });

    m_server.set_close_handler([this](connection_hdl hdl) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_connections.erase(hdl);
        for (auto& pair : m_subscriptions) {
            pair.second.erase(hdl);
        }
        logger.log(Logger::LogLevel::INFO, "Client disconnected");
    });

    m_server.set_message_handler([this](connection_hdl hdl, server::message_ptr msg) {
        on_message(hdl, msg);
    });

    m_deribit_client.set_broadcast_callback([this](const std::string& channel, const std::string& data) {
        PerformanceTracker t("broadcast_orderbook");
        logger.log(Logger::LogLevel::INFO, "Received broadcast from Deribit");
        broadcast_orderbook(channel, data);
        t.stop();
    });
}

void WebSocketServer::run(uint16_t port) {
    if (m_running) return;

    try {
        m_server.listen(port);
        m_server.start_accept();
    
        m_deribit_client.connect_websocket();
        
        m_running = true;
        m_server.run();
    } catch (const std::exception& e) {
        logger.log(Logger::LogLevel::ERROR, "Error running WebSocket server: " + std::string(e.what()));
        m_running = false;
        throw;
    }
}

void WebSocketServer::stop() {
    if (!m_running) return;

    try {
        m_server.stop();
        m_running = false;
    } catch (const std::exception& e) {
        logger.log(Logger::LogLevel::ERROR, "Error stopping WebSocket server: " + std::string(e.what()));
    }
}

bool WebSocketServer::is_running() const {
    return m_running;
}

void WebSocketServer::on_message(connection_hdl hdl, server::message_ptr msg) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    try {
        std::string payload = msg->get_payload();
        logger.log(Logger::LogLevel::INFO, "Received message: " + payload);
        
        json request = json::parse(payload);
        if (request.contains("action") && request["action"] == "subscribe" && 
            request.contains("symbol")) {
            
            std::string symbol = request["symbol"];
            handle_subscription(hdl, symbol);
        }
    } catch (const json::exception& e) {
        logger.log(Logger::LogLevel::ERROR, "Failed to parse WebSocket message: " + std::string(e.what()));
    }
}

void WebSocketServer::handle_subscription(connection_hdl hdl, const std::string& symbol) {
    std::string channel = "book." + symbol + ".agg2";
    
    m_subscriptions[channel].insert(hdl);
    
    if (m_subscriptions[channel].size() == 1) {
        std::cout << "First subscriber for " << symbol << ", subscribing to Deribit" << std::endl;
        m_deribit_client.subscribe_to_channel(channel);
    }
    
    std::cout << "Client subscribed to: " << symbol << std::endl;
    std::cout << "Total subscribers for " << symbol << ": " << m_subscriptions[channel].size() << std::endl;
    
    send_subscription_confirmation(hdl, channel);
}

void WebSocketServer::send_subscription_confirmation(connection_hdl hdl, const std::string& symbol) {
    json response = {
        {"status", "subscribed"},
        {"symbol", symbol}
    };
    
    try {
        logger.log(Logger::LogLevel::INFO, "Sending confirmation to client");
        m_server.send(hdl, response.dump(), websocketpp::frame::opcode::text);
    } catch (const std::exception& e) {
        logger.log(Logger::LogLevel::ERROR, "Error sending subscription confirmation: " + std::string(e.what()));
    }
}

void WebSocketServer::broadcast_orderbook(const std::string& symbol, const std::string& orderbook_update) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::cout << "Broadcasting to subscribers of " << symbol << std::endl;
    
    if (m_subscriptions.count(symbol)) {
        std::cout << "Found " << m_subscriptions[symbol].size() << " subscribers" << std::endl;
        for (const auto& hdl : m_subscriptions[symbol]) {
            try {
                m_server.send(hdl, orderbook_update, websocketpp::frame::opcode::text);
                logger.log(Logger::LogLevel::INFO, "Broadcasted orderbook update to client");
            } catch (const std::exception& e) {
                logger.log(Logger::LogLevel::ERROR, "Error broadcasting orderbook update: " + std::string(e.what()));
            }
        }
    } else {
        std::cout << "No subscribers found for " << symbol << std::endl;
    }
}
// websocket_manager.cpp
#include "websocket_manager.hpp"
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

WebSocketManager::WebSocketManager(DeribitClient& deribit_client)
    : m_running(false), m_deribit_client(deribit_client) {

    // Configure WebSocket server
    m_server.clear_access_channels(websocketpp::log::alevel::frame_payload);
    m_server.init_asio();

    // Set up connection handlers
    m_server.set_open_handler([this](connection_hdl hdl) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_connections.insert(hdl);
        std::cout << "Client connected!" << std::endl;
    });

    m_server.set_close_handler([this](connection_hdl hdl) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_connections.erase(hdl);
        for (auto& pair : m_subscriptions) {
            pair.second.erase(hdl);
        }
        std::cout << "Client disconnected!" << std::endl;
    });

    m_server.set_message_handler([this](connection_hdl hdl, server::message_ptr msg) {
        on_message(hdl, msg);
    });

    // Set up broadcast callback for Deribit client
    m_deribit_client.set_broadcast_callback([this](const std::string& channel, const std::string& data) {
        std::cout << "Received broadcast callback for channel: " << channel << std::endl;
        broadcast_orderbook(channel, data);
    });
}

void WebSocketManager::run(uint16_t port) {
    // std::cout << "Starting WebSocket server on port " << port << std::endl;
    if (m_running) return;
    // std::cout << "Starting WebSocket server on port " << port << std::endl;

    try {
        m_server.listen(port);
        m_server.start_accept();
        
        // Connect to Deribit
        m_deribit_client.connect_websocket();
        
        m_running = true;
        m_server.run();
    } catch (const std::exception& e) {
        std::cerr << "WebSocket server error: " << e.what() << std::endl;
        m_running = false;
        throw;
    }
}

void WebSocketManager::stop() {
    if (!m_running) return;

    try {
        m_server.stop();
        m_running = false;
    } catch (const std::exception& e) {
        std::cerr << "Error stopping WebSocket server: " << e.what() << std::endl;
    }
}

bool WebSocketManager::is_running() const {
    return m_running;
}

void WebSocketManager::on_message(connection_hdl hdl, server::message_ptr msg) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    try {
        std::string payload = msg->get_payload();
        std::cout << "Received from client: " << payload << std::endl;
        
        json request = json::parse(payload);
        if (request.contains("action") && request["action"] == "subscribe" && 
            request.contains("symbol")) {
            
            std::string symbol = request["symbol"];
            handle_subscription(hdl, symbol);
        }
    } catch (const json::exception& e) {
        std::cerr << "Invalid message format: " << e.what() << std::endl;
    }
}

void WebSocketManager::handle_subscription(connection_hdl hdl, const std::string& symbol) {
    std::string channel = "book." + symbol + ".agg2";
    
    m_subscriptions[channel].insert(hdl);
    
    // Subscribe to Deribit if this is the first subscriber
    if (m_subscriptions[channel].size() == 1) {
        std::cout << "First subscriber for " << symbol << ", subscribing to Deribit" << std::endl;
        m_deribit_client.subscribe_to_channel(channel);
    }
    
    std::cout << "Client subscribed to: " << symbol << std::endl;
    std::cout << "Total subscribers for " << symbol << ": " << m_subscriptions[channel].size() << std::endl;
    
    send_subscription_confirmation(hdl, channel);
}

void WebSocketManager::send_subscription_confirmation(connection_hdl hdl, const std::string& symbol) {
    json response = {
        {"status", "subscribed"},
        {"symbol", symbol}
    };
    
    try {
        std::cout << "Sending confirmation to client" << std::endl;
        m_server.send(hdl, response.dump(), websocketpp::frame::opcode::text);
    } catch (const std::exception& e) {
        std::cerr << "Error sending confirmation: " << e.what() << std::endl;
    }
}

void WebSocketManager::broadcast_orderbook(const std::string& symbol, const std::string& orderbook_update) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::cout << "Broadcasting to subscribers of " << symbol << std::endl;
    
    if (m_subscriptions.count(symbol)) {
        std::cout << "Found " << m_subscriptions[symbol].size() << " subscribers" << std::endl;
        for (const auto& hdl : m_subscriptions[symbol]) {
            try {
                m_server.send(hdl, orderbook_update, websocketpp::frame::opcode::text);
                std::cout << "Sent update to a subscriber" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error broadcasting to client: " << e.what() << std::endl;
            }
        }
    } else {
        std::cout << "No subscribers found for " << symbol << std::endl;
    }
}
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <mutex>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

typedef websocketpp::server<websocketpp::config::asio> server;
typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
using websocketpp::connection_hdl;

struct connection_hash {
    size_t operator()(websocketpp::connection_hdl const& hdl) const {
        return std::hash<void*>()(hdl.lock().get());
    }
};

struct connection_equal {
    bool operator()(websocketpp::connection_hdl const& hdl1, websocketpp::connection_hdl const& hdl2) const {
        return !(hdl1.owner_before(hdl2) || hdl2.owner_before(hdl1));
    }
};

class DeribitClient {
public:
    DeribitClient(const std::string& uri, const std::string& client_id, const std::string& client_secret)
        : m_uri(uri), m_client_id(client_id), m_client_secret(client_secret) {
        
        // Set logging level for debugging
        // m_client.set_access_channels(websocketpp::log::alevel::all);
        m_client.clear_access_channels(websocketpp::log::alevel::frame_payload);
        
        m_client.init_asio();
        m_client.set_tls_init_handler([](websocketpp::connection_hdl) {
            return std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12);
        });

        m_client.set_message_handler([this](websocketpp::connection_hdl, client::message_ptr msg) {
            on_message(msg);
        });

        m_client.set_open_handler([this](websocketpp::connection_hdl hdl) {
            m_hdl = hdl;
            std::cout << "Deribit connection opened!" << std::endl;
            authenticate();
        });

        m_client.set_fail_handler([](websocketpp::connection_hdl hdl) {
            std::cout << "Deribit connection failed!" << std::endl;
        });

        m_client.set_close_handler([](websocketpp::connection_hdl hdl) {
            std::cout << "Deribit connection closed!" << std::endl;
        });
    }

    void connect() {
        websocketpp::lib::error_code ec;
        auto con = m_client.get_connection(m_uri, ec);
        if (ec) {
            std::cerr << "Connection error: " << ec.message() << std::endl;
            return;
        }
        m_client.connect(con);
        m_client_thread = std::thread([this]() { 
            try {
                m_client.run(); 
            } catch (const std::exception& e) {
                std::cerr << "Deribit client thread error: " << e.what() << std::endl;
            }
        });
    }

    void subscribe_to_channel(const std::string& channel) {
        json subscribe_msg = {
            {"jsonrpc", "2.0"},
            {"id", 42},
            {"method", "public/subscribe"},
            {"params", {
                {"channels", {channel}}
            }}
        };
        std::cout << "Subscribing to channel: " << channel << std::endl;
        send_message(subscribe_msg);
    }

    void set_broadcast_callback(std::function<void(const std::string&, const std::string&)> callback) {
        m_broadcast_callback = callback;
    }

private:
    void authenticate() {
        json auth_msg = {
            {"jsonrpc", "2.0"},
            {"id", 9929},
            {"method", "public/auth"},
            {"params", {
                {"grant_type", "client_credentials"},
                {"client_id", m_client_id},
                {"client_secret", m_client_secret}
            }}
        };
        send_message(auth_msg);
    }

    void send_message(const json& msg) {
        if (m_hdl.lock()) {
            std::string message = msg.dump();
            std::cout << "Sending to Deribit: " << message << std::endl;
            try {
                m_client.send(m_hdl, message, websocketpp::frame::opcode::text);
            } catch (const std::exception& e) {
                std::cerr << "Error sending message to Deribit: " << e.what() << std::endl;
            }
        } else {
            std::cerr << "Cannot send message - no valid connection handle" << std::endl;
        }
    }

    void on_message(client::message_ptr msg) {
        // std::cout << msg->get_payload() << std::endl;
        try {
            std::string payload = msg->get_payload();
            // std::cout << "Received from Deribit: " << payload << std::endl;
            
            json response = json::parse(payload);
            
            if (response.contains("method") && response["method"] == "heartbeat") {
                std::cout << "Received heartbeat!" << std::endl;
                json heartbeat_response = {
                    {"jsonrpc", "2.0"},
                    {"method", "public/test"}
                };
                send_message(heartbeat_response);
            }

            if (response.contains("id")) {
                int id = response["id"].get<int>();
                if (id == 9929 && response.contains("result")) {
                    std::cout << "Authentication successful!" << std::endl;
                    json heartbeat_msg = {
                        {"jsonrpc", "2.0"},
                        {"id", 9098},
                        {"method", "public/set_heartbeat"},
                        {"params", {{"interval", 100}}}
                    };
                    std::cout << "Setting heartbeat interval to 100s" << std::endl;
                    send_message(heartbeat_msg);
                } else if (id == 42 && response.contains("result")) {
                    std::cout << "Received from Deribit: ";
                    std::cout << response.dump(2) << std::endl;
                    std::cout << "Successfully subscribed to channel!" << std::endl;
                }
            }

            // Forward market data to subscribers
            if (response.contains("params") && response["params"].contains("channel")) {
                std::string channel = response["params"]["channel"];
                std::string data = msg->get_payload(); // Send the entire message instead of just the data
                std::cout << "Broadcasting channel update: " << channel << std::endl;
                if (m_broadcast_callback) {
                    std::cout << "Broadcast callback set!" << std::endl;
                    m_broadcast_callback(channel, data);
                } else {
                    std::cout << "No broadcast callback set!" << std::endl;
                }
            }
        } catch (const json::exception& e) {
            std::cerr << "Failed to parse message: " << e.what() << std::endl;
        }
    }

    client m_client;
    websocketpp::connection_hdl m_hdl;
    std::string m_uri;
    std::string m_client_id;
    std::string m_client_secret;
    std::thread m_client_thread;
    std::function<void(const std::string&, const std::string&)> m_broadcast_callback;
};

class WebSocketServer {
public:
    WebSocketServer(const std::string& deribit_uri, 
                   const std::string& client_id, 
                   const std::string& client_secret) 
        : m_deribit_client(deribit_uri, client_id, client_secret) {
        
        // Set logging level for debugging
        // m_server.set_access_channels(websocketpp::log::alevel::all);
        m_server.clear_access_channels(websocketpp::log::alevel::frame_payload);
        
        m_server.init_asio();

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

        // Set up broadcast callback
        m_deribit_client.set_broadcast_callback([this](const std::string& channel, const std::string& data) {
            std::cout << "Received broadcast callback for channel: " << channel << std::endl;
            broadcast_orderbook(channel, data);
        });
    }

    void run(uint16_t port) {
        m_server.listen(port);
        m_server.start_accept();
        
        // Connect to Deribit
        m_deribit_client.connect();
        
        // Run the server
        try {
            m_server.run();
        } catch (const std::exception& e) {
            std::cerr << "Server error: " << e.what() << std::endl;
        }
    }

private:
    void on_message(connection_hdl hdl, server::message_ptr msg) {
        std::lock_guard<std::mutex> lock(m_mutex);
        try {
            std::string payload = msg->get_payload();
            std::cout << "Received from client: " << payload << std::endl;
            
            json request = json::parse(payload);
            if (request.contains("action") && request["action"] == "subscribe" && request.contains("symbol")) {
                std::string symbol = request["symbol"];
                std::string channel = "book."+symbol+".raw";
                
                m_subscriptions[channel].insert(hdl);
                // If this is the first subscriber, subscribe to Deribit
                if (m_subscriptions[channel].size() == 1) {
                    std::cout << "First subscriber for " << symbol << ", subscribing to Deribit" << std::endl;
                    m_deribit_client.subscribe_to_channel(channel);
                } else {
                // Add the client to subscribers
                }
                std::cout << "Client subscribed to: " << symbol << std::endl;
                std::cout << "Total subscribers for " << symbol << ": " << m_subscriptions[channel].size() << std::endl;
                
                // Send confirmation to client
                json response = {
                    {"status", "subscribed"},
                    {"symbol", channel}
                };
                try {
                    std::cout << "Sending confirmation to client" << std::endl;
                    m_server.send(hdl, response.dump(), websocketpp::frame::opcode::text);
                } catch (const std::exception& e) {
                    std::cerr << "Error sending confirmation: " << e.what() << std::endl;
                }
            }
        } catch (const json::exception& e) {
            std::cerr << "Invalid message format: " << e.what() << std::endl;
        }
    }

    void broadcast_orderbook(const std::string& symbol, const std::string& orderbook_update) {
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

    server m_server;
    DeribitClient m_deribit_client;
    std::mutex m_mutex;
    std::unordered_set<connection_hdl, connection_hash, connection_equal> m_connections;
    std::unordered_map<std::string, std::unordered_set<connection_hdl, connection_hash, connection_equal>> m_subscriptions;
};

int main() {
    const std::string uri = "wss://test.deribit.com/ws/api/v2";
    const std::string client_id = "ul416k44";
    const std::string client_secret = "skRvQdug7OWkYzc9WL4vOkrk2vGoRO0B-UyRRezde3A";


    try {
        WebSocketServer server(uri, client_id, client_secret);
        server.run(9002);
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
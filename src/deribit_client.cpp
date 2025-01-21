// deribit_client.cpp
#include "deribit_client.hpp"
#include "config.h"

DeribitClient::DeribitClient() : m_ws_enabled(true) {
    this->client_id = CLIENT_ID;
    this->client_secret = CLIENT_SECRET;
    this->base_url = BASE_URL;
    this->m_ws_uri = WEB_SOCKET_URL;
    if (m_ws_enabled) {
        init_websocket();
    }
}

DeribitClient::DeribitClient(DeribitClient& other) {
    this->client_id = other.client_id;
    this->client_secret = other.client_secret;
    this->access_token = other.access_token;
    this->refresh_token = other.refresh_token;
    this->base_url = other.base_url;
    this->m_ws_uri = other.m_ws_uri;
    this->m_ws_enabled = other.m_ws_enabled;
    if (m_ws_enabled) {
        init_websocket();
    }
}

// DeribitClient::DeribitClient(const std::string& ws_uri, const std::string& client_id, 
//                            const std::string& client_secret) 
//     : m_ws_uri(ws_uri), client_id(client_id), client_secret(client_secret), m_ws_enabled(true) {
//     this->base_url = BASE_URL;
//     if (m_ws_enabled) {
//         init_websocket();
//     }
// }

DeribitClient::~DeribitClient() {
    if (m_ws_enabled && m_client_thread.joinable()) {
        m_client.stop();
        m_client_thread.join();
    }
}

void DeribitClient::init_websocket() {
    m_client.clear_access_channels(websocketpp::log::alevel::frame_payload);
    m_client.init_asio();
    
    m_client.set_tls_init_handler([](websocketpp::connection_hdl) {
        return std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12);
    });

    m_client.set_message_handler([this](websocketpp::connection_hdl, ws_client::message_ptr msg) {
        on_websocket_message(msg);
    });

    m_client.set_open_handler([this](websocketpp::connection_hdl hdl) {
        m_ws_hdl = hdl;
        std::cout << "WebSocket connection opened" << std::endl;
        websocket_authenticate();
    });

    m_client.set_fail_handler([](websocketpp::connection_hdl) {
        std::cout << "WebSocket connection failed" << std::endl;
    });

    m_client.set_close_handler([](websocketpp::connection_hdl) {
        std::cout << "WebSocket connection closed" << std::endl;
    });
}

void DeribitClient::connect_websocket() {
    websocketpp::lib::error_code ec;
    auto conn = m_client.get_connection(m_ws_uri, ec);
    if (ec) {
        std::cerr << "WebSocket connection error: " << ec.message() << std::endl;
        return;
    }

    m_client.connect(conn);
    m_client_thread = std::thread([this]() {
        try {
            m_client.run();
        } catch (const std::exception& e) {
            std::cerr << "WebSocket client thread error: " << e.what() << std::endl;
        }
    });
}

void DeribitClient::subscribe_to_channel(const std::string& channel) {
    if (!m_ws_enabled) return;
    
    nlohmann::json subscribe_msg = {
        {"jsonrpc", "2.0"},
        {"id", 42},
        {"method", "public/subscribe"},
        {"params", {
            {"channels", {channel}}
        }}
    };
    send_websocket_message(subscribe_msg);
}

void DeribitClient::send_websocket_message(const nlohmann::json& msg) {
    if (!m_ws_enabled) return;

    if (auto hdl = m_ws_hdl.lock()) {
        try {
            m_client.send(hdl, msg.dump(), websocketpp::frame::opcode::text);
        } catch (const std::exception& e) {
            std::cerr << "Error sending WebSocket message: " << e.what() << std::endl;
        }
    }
}

void DeribitClient::websocket_authenticate() {
    nlohmann::json auth_msg = {
        {"jsonrpc", "2.0"},
        {"id", 9929},
        {"method", "public/auth"},
        {"params", {
            {"grant_type", "client_credentials"},
            {"client_id", client_id},
            {"client_secret", client_secret}
        }}
    };
    send_websocket_message(auth_msg);
}

void DeribitClient::on_websocket_message(ws_client::message_ptr msg) {
    try {
        std::string payload = msg->get_payload();
        nlohmann::json response = nlohmann::json::parse(payload);
        
        if (response.contains("method") && response["method"] == "heartbeat") {
            nlohmann::json heartbeat_response = {
                {"jsonrpc", "2.0"},
                {"method", "public/test"}
            };
            send_websocket_message(heartbeat_response);
        }

        if (response.contains("id")) {
            int id = response["id"].get<int>();
            if (id == 9929 && response.contains("result")) {
                std::cout << "WebSocket authentication successful" << std::endl;
                nlohmann::json heartbeat_msg = {
                    {"jsonrpc", "2.0"},
                    {"id", 9098},
                    {"method", "public/set_heartbeat"},
                    {"params", {{"interval", 100}}}
                };
                send_websocket_message(heartbeat_msg);
            }
        }

        if (response.contains("params") && response["params"].contains("channel")) {
            std::string channel = response["params"]["channel"];
            if (m_broadcast_callback) {
                m_broadcast_callback(channel, payload);
            }
        }
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "Failed to parse WebSocket message: " << e.what() << std::endl;
    }
}

void DeribitClient::set_broadcast_callback(std::function<void(const std::string&, const std::string&)> callback) {
    m_broadcast_callback = callback;
}

// Keep all existing REST API methods (authenticate, place_buy_order, place_sell_order, etc.) as they are

cpr::Response DeribitClient::authenticate() {
    nlohmann::json payload = {
            {"jsonrpc", "2.0"},
            {"method", "public/auth"},
            {"params", {
                {"grant_type", "client_credentials"},
                {"client_id", client_id},
                {"client_secret", client_secret}
            }},
            {"id", 1}
    };
    cpr::Response r = post(payload);
    if (r.status_code == 200) {
        nlohmann::json response = nlohmann::json::parse(r.text);
        this->access_token = response["result"]["access_token"];
        this->refresh_token = response["result"]["refresh_token"];
        // std::cout << this << std::endl;
        return r;
    }
    std::cerr << "Authentication failed. HTTP Code: " << r.status_code << "\nResponse: " << r.text << std::endl;
    throw std::runtime_error("Authentication failed.");
}

cpr::Response DeribitClient::place_buy_order(const std::string& instrument_name, const std::string& side, const std::string& type, const std::string& amount, const std::string& price) {
    nlohmann::json payload = {
            {"jsonrpc", "2.0"},
            {"method", "private/buy"},
            {"params", {
                {"instrument_name", instrument_name},
                {"amount", amount},
                {"type", type},
                {"price", price},
                {"label", "label"}
            }},
            {"id", 1}
    };
    return post(payload, true);
}
cpr::Response DeribitClient::place_sell_order(const std::string& instrument_name, const std::string& side, const std::string& type, const std::string& amount, const std::string& price) {
    nlohmann::json payload = {
            {"jsonrpc", "2.0"},
            {"method", "private/buy"},
            {"params", {
                {"instrument_name", instrument_name},
                {"amount", amount},
                {"type", type},
                {"price", price},
                {"label", "label"}
            }},
            {"id", 1}
    };
    return post(payload, true);
}

cpr::Response DeribitClient::get_positions() {
    nlohmann::json payload = {
            {"jsonrpc", "2.0"},
            {"method", "private/get_positions"},
            {"params", {}},
            {"id", 1}
    };
    return get(payload);
}

cpr::Response DeribitClient::get_order_book(const std::string& instrument_name) {
    nlohmann::json payload = {
            {"jsonrpc", "2.0"},
            {"method", "public/get_order_book"},
            {"params", {
                {"instrument_name", instrument_name},
                {"depth", 1}
            }},
            {"id", 1}
    };
    return post(payload);
}


cpr::Response DeribitClient::cancel_order(const std::string& order_id) {
    nlohmann::json payload = {
            {"jsonrpc", "2.0"},
            {"method", "private/cancel_order"},
            {"params", {
                {"order_id", order_id}
            }},
            {"id", 1}
    };
    return post(payload, true);
}

cpr::Response DeribitClient::edit_order(const std::string& order_id, const std::string& quantity, const std::string& price) {
    nlohmann::json payload = {
            {"jsonrpc", "2.0"},
            {"method", "private/edit_order"},
            {"params", {
                {"order_id", order_id},
                {"amount", quantity},
                {"price", price}
            }},
            {"id", 1}
    };
    return get(payload);
}

cpr::Response DeribitClient::post(const nlohmann::json& payload, bool with_auth) {
    if (with_auth) {
        return cpr::Post(cpr::Url{BASE_URL},
                        cpr::Body{payload.dump()},
                        cpr::Header{{"Authorization", "Bearer " + this->access_token}}
        );
    } else {
        return cpr::Post(cpr::Url{BASE_URL},
                        cpr::Body{payload.dump()}
        );
    }
}

cpr::Response DeribitClient::get(const nlohmann::json& payload) {
    return cpr::Get(cpr::Url{BASE_URL},
                    cpr::Body{payload.dump()},
                    cpr::Header{{"Authorization", "Bearer " + this->access_token}}
    );
}

// overloading the << operator to print the client object
std::ostream& operator<<(std::ostream& os, const DeribitClient& client) {
    os << "Client ID: " << client.client_id << std::endl;
    os << "Client Secret: " << client.client_secret << std::endl;
    os << "Access Token: " << client.access_token << std::endl;
    os << "Refresh Token: " << client.refresh_token << std::endl;
    return os;
}




#ifndef DERIBIT_CLIENT_HPP
#define DERIBIT_CLIENT_HPP

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <functional>

class DeribitClient {
public:
    DeribitClient();
    DeribitClient(DeribitClient& other);
    ~DeribitClient();

    // REST API methods
    cpr::Response authenticate();
    cpr::Response place_buy_order(const std::string& instrument_name, const std::string& side, 
                                 const std::string& type, const std::string& amount, 
                                 const std::string& price);
    cpr::Response place_sell_order(const std::string& instrument_name, const std::string& side, 
                                  const std::string& type, const std::string& amount, 
                                  const std::string& price);
    cpr::Response get_positions(const std::string& currency, const std::string& kind);
    cpr::Response get_order_book(const std::string& instrument_name);
    cpr::Response cancel_order(const std::string& order_id);
    cpr::Response edit_order(const std::string& order_id, const std::string& quantity, 
                            const std::string& price);

    // WebSocket methods
    void connect_websocket();
    void subscribe_to_channel(const std::string& channel);
    void set_broadcast_callback(std::function<void(const std::string&, const std::string&)> callback);
    bool is_websocket_connected() const { return m_ws_hdl.lock() != nullptr; }
    
    friend std::ostream& operator<<(std::ostream& os, const DeribitClient& client);
    

private:
    // REST API helpers
    cpr::Response post(const nlohmann::json& payload, bool with_auth = false);
    cpr::Response get(const nlohmann::json& payload);

    // Common members
    std::string client_id;
    std::string client_secret;
    std::string base_url;
    std::string access_token;
    std::string refresh_token;

    // WebSocket members
    typedef websocketpp::client<websocketpp::config::asio_tls_client> ws_client;
    ws_client m_client;
    websocketpp::connection_hdl m_ws_hdl;
    std::string m_ws_uri;
    std::thread m_client_thread;
    std::function<void(const std::string&, const std::string&)> m_broadcast_callback;
    bool m_ws_enabled;

    // WebSocket helpers
    void init_websocket();
    void websocket_authenticate();
    void send_websocket_message(const nlohmann::json& msg);
    void on_websocket_message(ws_client::message_ptr msg);
};

#endif

#ifndef WEBSOCKET_MANAGER_HPP
#define WEBSOCKET_MANAGER_HPP

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <mutex>
#include "deribit_client.hpp"

typedef websocketpp::server<websocketpp::config::asio> server;
typedef websocketpp::connection_hdl connection_hdl;

struct connection_hash {
    size_t operator()(websocketpp::connection_hdl const& hdl) const {
        return std::hash<void*>()(hdl.lock().get());
    }
};

struct connection_equal {
    bool operator()(websocketpp::connection_hdl const& hdl1, 
                   websocketpp::connection_hdl const& hdl2) const {
        return !(hdl1.owner_before(hdl2) || hdl2.owner_before(hdl1));
    }
};

class WebSocketServer {
public:
    WebSocketServer(DeribitClient& m_deribit_client);

    void run(uint16_t port);
    void stop();
    bool is_running() const;

private:
    void on_message(connection_hdl hdl, server::message_ptr msg);
    void broadcast_orderbook(const std::string& symbol, const std::string& orderbook_update);
    void handle_subscription(connection_hdl hdl, const std::string& symbol);
    void send_subscription_confirmation(connection_hdl hdl, const std::string& symbol);

    server m_server;
    DeribitClient m_deribit_client;
    std::mutex m_mutex;
    std::unordered_set<connection_hdl, connection_hash, connection_equal> m_connections;
    std::unordered_map<std::string, 
        std::unordered_set<connection_hdl, connection_hash, connection_equal>> m_subscriptions;
    bool m_running;
};

#endif
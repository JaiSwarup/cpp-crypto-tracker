#include "order_manager.hpp"
#include "deribit_client.hpp"

OrderManager::OrderManager(DeribitClient client) : client(client) {}

OrderManager::~OrderManager() {}

std::string OrderManager::view_current_positions() {
    return client.get_positions().text;
}

std::string OrderManager::get_orderbook(const std::string& order_id) {
    return client.get_order_book(order_id).text;
}

std::string OrderManager::place_order(const std::string& symbol, const std::string& side, const std::string& type, const std::string& quantity, const std::string& price) {
    if (side == "buy") {
        return client.place_buy_order(symbol, side, type, quantity, price).text;
    } else {
        return client.place_sell_order(symbol, side, type, quantity, price).text;
    }
}

std::string OrderManager::cancel_order(const std::string& order_id) {
    return client.cancel_order(order_id).text;
}

std::string OrderManager::modify_order(const std::string& order_id, const std::string& quantity, const std::string& price) {
    return client.edit_order(order_id, quantity, price).text;
}

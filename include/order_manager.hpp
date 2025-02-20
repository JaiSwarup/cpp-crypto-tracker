#ifndef ORDER_MANAGER
#define ORDER_MANAGER

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "logger.hpp"
#include "deribit_client.hpp"

class OrderManager {
    public:
        OrderManager(DeribitClient client);
        ~OrderManager();
        std::string view_current_positions(const std::string& currency, const std::string& kind);
        std::string get_orderbook(const std::string& instrument_name);
        std::string place_order(const std::string& symbol, const std::string& side, const std::string& type, const std::string& quantity, const std::string& price);
        std::string cancel_order(const std::string& order_id);
        std::string modify_order(const std::string& order_id, const std::string& quantity, const std::string& price);
    private:
        DeribitClient client;
        Logger logger;
};

#endif
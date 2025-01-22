#include "order_manager.hpp"
#include "deribit_client.hpp"
#include "logger.hpp"
#include "performance_tracker.hpp"
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

using json = nlohmann::json;
using response = cpr::Response;

OrderManager::OrderManager(DeribitClient client) : client(client) {
    this->logger = Logger();
}

OrderManager::~OrderManager() {}

std::string OrderManager::view_current_positions(const std::string& currency, const std::string& kind) {
    logger.log(Logger::LogLevel::INFO, "Viewing current positions");
    try {
        PerformanceTracker tracker("view_current_positions");
        response r = client.get_positions(currency, kind);
        tracker.stop();

        json j = json::parse(r.text);
        if (j.contains("error")){
            logger.log(Logger::LogLevel::WARNING, j["error"]["message"]);
            return j["error"].dump();
        } else {
            logger.log(Logger::LogLevel::SUCCESS, "Positions retrieved successfully");
            return j["result"].dump(4);
        }
    } catch (const std::exception& e) {
        logger.log(Logger::LogLevel::ERROR, e.what());
        return "";
    }
}

std::string OrderManager::get_orderbook(const std::string& instrument_name) {
    logger.log(Logger::LogLevel::INFO, "Getting orderbook");
    try {
        PerformanceTracker tracker("get_orderbook");
        response r = client.get_order_book(instrument_name);
        tracker.stop();
        
        json j = json::parse(r.text);
        if (j.contains("error")){
            logger.log(Logger::LogLevel::WARNING, j["error"]["message"]);
            return j["error"].dump();
        }
        logger.log(Logger::LogLevel::SUCCESS, "Orderbook retrieved successfully");
        return j["result"].dump(4);
    } catch (const std::exception& e) {
        logger.log(Logger::LogLevel::ERROR, e.what());
    }
    return "";
}

std::string OrderManager::place_order(const std::string& symbol, const std::string& side, const std::string& type, const std::string& quantity, const std::string& price) {
    if (side == "buy") {
            logger.log(Logger::LogLevel::INFO, "Placing buy order");
        try {
            PerformanceTracker tracker("place_order: buy");
            response r = client.place_buy_order(symbol, side, type, quantity, price);
            tracker.stop();

            json j = json::parse(r.text);
            if (j.contains("error")){
                logger.log(Logger::LogLevel::WARNING, j["error"]["message"]);
                return j["error"].dump();
            }
            logger.log(Logger::LogLevel::SUCCESS, "Order placed successfully");
            return j["result"].dump(4);
        } catch (const std::exception& e) {
            logger.log(Logger::LogLevel::ERROR, e.what());
        }
            return "";
    } else {
        logger.log(Logger::LogLevel::INFO, "Placing sell order");
        try {
            PerformanceTracker tracker("place_order: sell");
            response r = client.place_sell_order(symbol, side, type, quantity, price);
            tracker.stop();

            json j = json::parse(r.text);
            if (j.contains("error")){
                logger.log(Logger::LogLevel::WARNING, j["error"]["message"]);
                return j["error"].dump();
            }
            logger.log(Logger::LogLevel::SUCCESS, "Order placed successfully");
            return j["result"].dump(4);
        } catch (const std::exception& e) {
            logger.log(Logger::LogLevel::ERROR, e.what());
        }
        return "";
    }
}

std::string OrderManager::cancel_order(const std::string& order_id) {
    logger.log(Logger::LogLevel::INFO, "Cancelling order");
    try {
        PerformanceTracker tracker("cancel_order");
        response r = client.cancel_order(order_id);
        tracker.stop();

        json j = json::parse(r.text);
        if (j.contains("error")){
            logger.log(Logger::LogLevel::WARNING, j["error"]["message"]);
            return j["error"].dump();
        }
        logger.log(Logger::LogLevel::SUCCESS, "Order cancelled successfully");
        return j["result"].dump(4);
    } catch (const std::exception& e) {
        logger.log(Logger::LogLevel::ERROR, e.what());
    }
        return "";
}

std::string OrderManager::modify_order(const std::string& order_id, const std::string& quantity, const std::string& price) {
    logger.log(Logger::LogLevel::INFO, "Modifying order");
    try {
        PerformanceTracker tracker("modify_order");
        response r = client.edit_order(order_id, quantity, price);
        tracker.stop();

        json j = json::parse(r.text);
        if (j.contains("error")){
            logger.log(Logger::LogLevel::WARNING, j["error"]["message"]);
            return j["error"].dump();
        }
        logger.log(Logger::LogLevel::SUCCESS, "Order modified successfully");
        return j["result"].dump(4);
    } catch (const std::exception& e) {
        logger.log(Logger::LogLevel::ERROR, e.what());
    }
        return "";
}

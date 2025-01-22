#include "order_manager.hpp"
#include "performance_tracker.hpp"
#include "config.h"
#include "market_manager.hpp"
#include <unordered_map>
#include <vector>
#include <numeric>

class PerformanceOrderManager {
public:
    PerformanceOrderManager(OrderManager& manager) : manager(manager) {}

    // Method to get performance statistics for a specific function
    struct PerformanceStats {
        double avg_time = 0.0;
        double min_time = std::numeric_limits<double>::max();
        double max_time = std::numeric_limits<double>::min();
        size_t call_count = 0;
    };

    // Get stats for a specific function
    PerformanceStats get_stats(const std::string& function_name) const {
        if (performance_data.find(function_name) != performance_data.end()) {
            return calculate_stats(performance_data.at(function_name));
        }
        return PerformanceStats{};
    }

    // Clear performance data
    void clear_stats() {
        performance_data.clear();
    }

    // Wrapped methods with performance tracking
    std::string view_current_positions(const std::string& currency, const std::string& kind) {
        return performance_wrapper("view_current_positions", [&]() {
            return manager.view_current_positions(currency, kind);
        });
    }

    std::string get_orderbook(const std::string& order_id) {
        return performance_wrapper("get_orderbook", [&]() {
            return manager.get_orderbook(order_id);
        });
    }

    std::string place_order(const std::string& symbol, const std::string& side, 
                           const std::string& type, const std::string& quantity, 
                           const std::string& price) {
        return performance_wrapper("place_order", [&]() {
            return manager.place_order(symbol, side, type, quantity, price);
        });
    }

    std::string cancel_order(const std::string& order_id) {
        return performance_wrapper("cancel_order", [&]() {
            return manager.cancel_order(order_id);
        });
    }

    std::string modify_order(const std::string& order_id, const std::string& quantity, 
                            const std::string& price) {
        return performance_wrapper("modify_order", [&]() {
            return manager.modify_order(order_id, quantity, price);
        });
    }

private:
    OrderManager& manager;
    std::unordered_map<std::string, std::vector<double>> performance_data;

    template<typename Func>
    std::string performance_wrapper(const std::string& funcName, Func&& func) {
        PerformanceTracker tracker(funcName);
        auto result = std::forward<Func>(func)();
        double elapsed = tracker.elapsed_time();
        
        // Store timing data
        if (performance_data.find(funcName) == performance_data.end()) {
            performance_data[funcName] = std::vector<double>();
        }
        performance_data[funcName].push_back(elapsed);
        
        return result;
    }

    PerformanceStats calculate_stats(const std::vector<double>& times) const {
        PerformanceStats stats;
        if (times.empty()) return stats;

        stats.call_count = times.size();
        stats.min_time = *std::min_element(times.begin(), times.end());
        stats.max_time = *std::max_element(times.begin(), times.end());
        stats.avg_time = std::accumulate(times.begin(), times.end(), 0.0) / times.size();

        return stats;
    }
};

class PerformanceMarketManager {
public:
    PerformanceMarketManager(MarketManager& manager) : manager(manager) {}

    // Method to get performance statistics for a specific function
    struct PerformanceStats {
        double avg_time = 0.0;
        double min_time = std::numeric_limits<double>::max();
        double max_time = std::numeric_limits<double>::min();
        size_t call_count = 0;
    };

    // Get stats for a specific function
    PerformanceStats get_stats(const std::string& function_name) const {
        if (performance_data.find(function_name) != performance_data.end()) {
            return calculate_stats(performance_data.at(function_name));
        }
        return PerformanceStats{};
    }

    // Clear performance data
    void clear_stats() {
        performance_data.clear();
    }

    // Wrapped methods with performance tracking
    std::string get_market_data(const std::string& currency, const std::string& kind) {
        return performance_wrapper("get_market_data", [&]() {
            return manager.view_all_instruments(currency, kind);
        });
    }
private:
    MarketManager& manager;
    std::unordered_map<std::string, std::vector<double>> performance_data;

    template<typename Func>
    std::string performance_wrapper(const std::string& funcName, Func&& func) {
        PerformanceTracker tracker(funcName);
        auto result = std::forward<Func>(func)();
        double elapsed = tracker.elapsed_time();
        
        // Store timing data
        if (performance_data.find(funcName) == performance_data.end()) {
            performance_data[funcName] = std::vector<double>();
        }
        performance_data[funcName].push_back(elapsed);
        
        return result;
    }

    PerformanceStats calculate_stats(const std::vector<double>& times) const {
        PerformanceStats stats;
        if (times.empty()) return stats;

        stats.call_count = times.size();
        stats.min_time = *std::min_element(times.begin(), times.end());
        stats.max_time = *std::max_element(times.begin(), times.end());
        stats.avg_time = std::accumulate(times.begin(), times.end(), 0.0) / times.size();

        return stats;
    }
};

int main (){
    // tests for bench marking requests
    loadConfig();
    DeribitClient deribit_client;
    deribit_client.authenticate();
    OrderManager order_manager_instance(deribit_client);
    MarketManager market_manager_instance(deribit_client);

    PerformanceOrderManager order_manager(order_manager_instance);
    PerformanceMarketManager market_manager(market_manager_instance);

    //tests
    std::vector<std::vector<std::string>> order_tests = {
        {"place_order", "BTC-PERPETUAL", "buy", "limit","1", "10000"},
        {"place_order", "BTC-PERPETUAL", "sell","limit", "1", "10000"},
        {"cancel_order", "1"},
        {"modify_order", "1", "1", "10000"},
        {"view_current_positions", "BTC", "call"},
        {"get_orderbook", "BTC"},
        {"view_current_positions", "BTC", "future"},
        {"get_orderbook", "BTC"},
        {"place_order", "ETH-PERPETUAL","buy", "market", "1", "10000"},
        {"place_order", "ETH-PERPETUAL","sell", "market", "1", "10000"},
        {"cancel_order", "1"},
        {"modify_order", "1", "1", "10000"},
        {"view_current_positions", "ETH", "call"},
        {"get_orderbook", "ETH"},
        {"view_current_positions", "ETH", "future"},
        {"get_orderbook", "ETH"},
    };

    std::vector<std::vector<std::string>> market_tests = {
        {"get_market_data", "BTC", "future"},
        {"get_market_data", "BTC", "option"},
        {"get_market_data", "USDC", "spot"},
        {"get_market_data", "EURR", "future"},
        {"get_market_data", "USDC", "option"},
        {"get_market_data", "ETH", "spot"},
        {"get_market_data", "BTC", "future"},
        {"get_market_data", "BTC", "option"},
        {"get_market_data", "USDC", "spot"},
        {"get_market_data", "EURR", "future"},
        {"get_market_data", "USDC", "option"},
        {"get_market_data", "ETH", "spot"},
    };

    for (const auto& test : order_tests) {
        try {

        if (test[0] == "place_order") {
            order_manager.place_order(test[1], test[2], test[3], test[4], test[5]);
        } else if (test[0] == "cancel_order") {
            order_manager.cancel_order(test[1]);
        } else if (test[0] == "modify_order") {
            order_manager.modify_order(test[1], test[2], test[3]);
        } else if (test[0] == "view_current_positions") {
            order_manager.view_current_positions(test[1], test[2]);
        } else if (test[0] == "get_orderbook") {
            order_manager.get_orderbook(test[1]);
        }
        }
        catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }

    for (const auto& test : market_tests) {
        try {
            if (test[0] == "get_market_data") {
                market_manager.get_market_data(test[1], test[2]);
            }
        }
        catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }

    std::cout << "Order Manager Performance Stats:" << std::endl;
    std::cout << "place_order: ";
    std::cout << "AverageTime" << order_manager.get_stats("place_order").avg_time << "ms" << std::endl;
    std::cout << "MaxTime" << order_manager.get_stats("place_order").max_time << "ms" << std::endl;
    std::cout << "MinTime" << order_manager.get_stats("place_order").min_time << "ms" << std::endl;
    std::cout << "cancel_order: ";
    std::cout << "AverageTime" << order_manager.get_stats("cancel_order").avg_time << "ms" << std::endl;
    std::cout << "MaxTime" << order_manager.get_stats("cancel_order").max_time << "ms" << std::endl;
    std::cout << "MinTime" << order_manager.get_stats("cancel_order").min_time << "ms" << std::endl;
    std::cout << "modify_order: ";
    std::cout << "AverageTime" << order_manager.get_stats("modify_order").avg_time << "ms" << std::endl;
    std::cout << "MaxTime" << order_manager.get_stats("modify_order").max_time << "ms" << std::endl;
    std::cout << "MinTime" << order_manager.get_stats("modify_order").min_time << "ms" << std::endl;
    std::cout << "view_current_positions: ";
    std::cout << "AverageTime" << order_manager.get_stats("view_current_positions").avg_time << "ms" << std::endl;
    std::cout << "MaxTime" << order_manager.get_stats("view_current_positions").max_time << "ms" << std::endl;
    std::cout << "MinTime" << order_manager.get_stats("view_current_positions").min_time << "ms" << std::endl;
    std::cout << "get_orderbook: ";
    std::cout << "AverageTime" << order_manager.get_stats("get_orderbook").avg_time << "ms" << std::endl;
    std::cout << "MaxTime" << order_manager.get_stats("get_orderbook").max_time << "ms" << std::endl;
    std::cout << "MinTime" << order_manager.get_stats("get_orderbook").min_time << "ms" << std::endl;



    std::cout << "Market Manager Performance Stats:" << std::endl;
    std::cout << "get_market_data: ";
    std::cout << "AverageTime" << market_manager.get_stats("get_market_data").avg_time << "ms" << std::endl;
    std::cout << "MaxTime" << market_manager.get_stats("get_market_data").max_time << "ms" << std::endl;
    std::cout << "MinTime" << market_manager.get_stats("get_market_data").min_time << "ms" << std::endl;

}
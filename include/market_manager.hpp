#ifndef MARKET_MANAGER_HPP
#define MARKET_MANAGER_HPP
#include "logger.hpp"
#include "deribit_client.hpp"

class MarketManager {
    public:
        MarketManager(DeribitClient client);
        ~MarketManager();
        std::string view_all_instruments(const std::string& currency, const std::string& kind);
    private:
        DeribitClient client;
        Logger logger;
};
#endif
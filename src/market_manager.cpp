#include "performance_tracker.hpp"
#include "market_manager.hpp"
#include <nlohmann/json.hpp>
#include <vector>
#include <cpr/cpr.h>

using json = nlohmann::json;
using response = cpr::Response;

MarketManager::MarketManager(DeribitClient client) : client(client) {
    this->logger = Logger();
}

MarketManager::~MarketManager() {}

std::string MarketManager::view_all_instruments(const std::string& currency, const std::string& kind) {
    logger.log(Logger::LogLevel::INFO, "Viewing all instruments");
    try {
        PerformanceTracker tracker("view_all_instruments");
        response r = client.get_all_instruments(currency, kind);
        tracker.stop();

        json j = json::parse(r.text);
        if (j.contains("error")){
            logger.log(Logger::LogLevel::WARNING, j["error"]["message"]);
            return j["error"].dump();
        } else {
            logger.log(Logger::LogLevel::SUCCESS, "Instruments retrieved successfully");
            std::vector<std::string> instruments;
            for (auto& instrument : j["result"]) {
                instruments.push_back(instrument["instrument_name"]);
            }

            return json(instruments).dump(4);
        }
    } catch (const std::exception& e) {
        logger.log(Logger::LogLevel::ERROR, e.what());
        return "";
    }
}
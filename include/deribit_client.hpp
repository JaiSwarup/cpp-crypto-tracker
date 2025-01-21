#ifndef DERIBIT_CLIENT
#define DERIBIT_CLIENT

#include <string>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

class DeribitClient {
    public:
        DeribitClient();
        ~DeribitClient();

        cpr::Response authenticate();
        cpr::Response refresh();

        cpr::Response place_buy_order(const std::string& instrument_name, const std::string& side, const std::string& type, const std::string& amount, const std::string& price);
        cpr::Response place_sell_order(const std::string& instrument_name, const std::string& side, const std::string& type, const std::string& amount, const std::string& price);
        cpr::Response get_positions();
        cpr::Response get_order_book(const std::string& order_id);
        cpr::Response cancel_order(const std::string& order_id);
        cpr::Response edit_order(const std::string& order_id, const std::string& quantity, const std::string& price);

        std::string get_access_token();
        std::string get_refresh_token();
        friend std::ostream& operator<<(std::ostream& os, const DeribitClient& client);
    private:
        std::string client_id;
        std::string client_secret;
        std::string base_url;
        std::string access_token;
        std::string refresh_token;

        cpr::Response post(const nlohmann::json& payload, bool auth_required = false);
        cpr::Response get(const nlohmann::json& payload);
};

#endif
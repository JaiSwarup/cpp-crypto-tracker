#ifndef DERIBIT_CLIENT
#define DERIBIT_CLIENT

#include <string>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

class DeribitClient {
    public:
        DeribitClient();
        ~DeribitClient();

        bool authenticate();
        bool refresh();

        cpr::Response place_order(const std::string& instrument_name, const std::string& side, const std::string& type, const double& amount, const double& price);

        std::string get_access_token();
        std::string get_refresh_token();

        friend std::ostream& operator<<(std::ostream& os, const DeribitClient& client);
    private:
        std::string client_id;
        std::string client_secret;
        std::string access_token;
        std::string refresh_token;
};

#endif
#include "deribit_client.hpp"
#include <cpr/cpr.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include "config.h"

DeribitClient::DeribitClient() {
    this->client_id = CLIENT_ID;
    this->client_secret = CLIENT_SECRET;
}
DeribitClient::~DeribitClient() {
}
bool DeribitClient::authenticate() {
    nlohmann::json payload = {
            {"jsonrpc", "2.0"},
            {"method", "public/auth"},
            {"params", {
                {"grant_type", "client_credentials"},
                {"client_id", client_id},
                {"client_secret", client_secret}
            }},
            {"id", 1}
    };
    cpr::Response r = cpr::Post(cpr::Url{"https://test.deribit.com/api/v2/public/auth"},
                                cpr::Body{payload.dump()}
    );
    if (r.status_code == 200) {
        nlohmann::json response = nlohmann::json::parse(r.text);
        this->access_token = response["result"]["access_token"];
        this->refresh_token = response["result"]["refresh_token"];
        std::cout << this << std::endl;
        return true;
    }
    // std::cout << r.text << std::endl;
    return false;
}

cpr::Response DeribitClient::place_order(const std::string& instrument_name, const std::string& side, const std::string& type, const double& amount, const double& price) {
    nlohmann::json payload = {
            {"jsonrpc", "2.0"},
            {"method", "private/buy"},
            {"params", {
                {"instrument_name", instrument_name},
                {"amount", amount},
                {"type", type},
                {"price", price},
                {"label", "label"}
            }},
            {"id", 1}
    };
    cpr::Response r = cpr::Post(cpr::Url{"https://test.deribit.com/api/v2/private/buy"},
                                cpr::Body{payload.dump()},
                                cpr::Header{{"Authorization", "Bearer " + this->access_token}}
    );
    return r;
}

// overloading the << operator to print the client object
std::ostream& operator<<(std::ostream& os, const DeribitClient& client) {
    os << "Client ID: " << client.client_id << std::endl;
    os << "Client Secret: " << client.client_secret << std::endl;
    os << "Access Token: " << client.access_token << std::endl;
    os << "Refresh Token: " << client.refresh_token << std::endl;
    return os;
}




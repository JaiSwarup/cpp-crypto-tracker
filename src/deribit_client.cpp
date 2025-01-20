#include "deribit_client.hpp"
#include <cpr/cpr.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include "config.h"

DeribitClient::DeribitClient() {
    this->client_id = CLIENT_ID;
    this->client_secret = CLIENT_SECRET;
    this->base_url = BASE_URL;
}
DeribitClient::~DeribitClient() {
}

cpr::Response DeribitClient::authenticate() {
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
    cpr::Response r = post(payload);
    if (r.status_code == 200) {
        nlohmann::json response = nlohmann::json::parse(r.text);
        this->access_token = response["result"]["access_token"];
        this->refresh_token = response["result"]["refresh_token"];
        std::cout << this << std::endl;
        return r;
    }
    std::cerr << "Authentication failed. HTTP Code: " << r.status_code << "\nResponse: " << r.text << std::endl;
    throw std::runtime_error("Authentication failed.");
}

cpr::Response DeribitClient::place_buy_order(const std::string& instrument_name, const std::string& side, const std::string& type, const std::string& amount, const std::string& price) {
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
    return post(payload, true);
}
cpr::Response DeribitClient::place_sell_order(const std::string& instrument_name, const std::string& side, const std::string& type, const std::string& amount, const std::string& price) {
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
    return post(payload, true);
}

cpr::Response DeribitClient::get_positions() {
    nlohmann::json payload = {
            {"jsonrpc", "2.0"},
            {"method", "private/get_positions"},
            {"params", {}},
            {"id", 1}
    };
    return get(payload);
}

cpr::Response DeribitClient::get_order_book(const std::string& instrument_name) {
    nlohmann::json payload = {
            {"jsonrpc", "2.0"},
            {"method", "public/get_order_book"},
            {"params", {
                {"instrument_name", instrument_name},
                {"depth", 1}
            }},
            {"id", 1}
    };
    return post(payload);
}


cpr::Response DeribitClient::cancel_order(const std::string& order_id) {
    nlohmann::json payload = {
            {"jsonrpc", "2.0"},
            {"method", "private/cancel_order"},
            {"params", {
                {"order_id", order_id}
            }},
            {"id", 1}
    };
    return post(payload, true);
}

cpr::Response DeribitClient::edit_order(const std::string& order_id, const std::string& quantity, const std::string& price) {
    nlohmann::json payload = {
            {"jsonrpc", "2.0"},
            {"method", "private/edit_order"},
            {"params", {
                {"order_id", order_id},
                {"amount", quantity},
                {"price", price}
            }},
            {"id", 1}
    };
    return get(payload);
}

cpr::Response DeribitClient::post(const nlohmann::json& payload, bool with_auth = false) {
    if (with_auth) {
        return cpr::Post(cpr::Url{BASE_URL},
                        cpr::Body{payload.dump()},
                        cpr::Header{{"Authorization", "Bearer " + this->access_token}}
        );
    } else {
        return cpr::Post(cpr::Url{BASE_URL},
                        cpr::Body{payload.dump()}
        );
    }
}

cpr::Response DeribitClient::get(const nlohmann::json& payload) {
    return cpr::Get(cpr::Url{BASE_URL},
                    cpr::Body{payload.dump()},
                    cpr::Header{{"Authorization", "Bearer " + this->access_token}}
    );
}

// overloading the << operator to print the client object
std::ostream& operator<<(std::ostream& os, const DeribitClient& client) {
    os << "Client ID: " << client.client_id << std::endl;
    os << "Client Secret: " << client.client_secret << std::endl;
    os << "Access Token: " << client.access_token << std::endl;
    os << "Refresh Token: " << client.refresh_token << std::endl;
    return os;
}




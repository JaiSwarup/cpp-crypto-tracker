#include <cpr/cpr.h>
#include <cstdlib>
#include "deribit_client.hpp"
#include "config.h"
#include <iostream>
#include <nlohmann/json.hpp>

int main(int argc, char** argv) {
    loadConfig();
    DeribitClient client;
    // std::cout << client << std::endl;
    client.authenticate();
    // std::cout << client << std::endl;

    return 0;
}
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
    // std::cout << client << std::endl;
    // std::cout << client.get_positions().text << std::endl;

    return 0;
}
#include "config.h"
#include "dotenv.h"
#include <iostream>

std::string CLIENT_ID;
std::string CLIENT_SECRET;
std::string BASE_URL;

void loadConfig() {
    if (!dotenv::load("../.env")) {
        std::cerr << "Failed to load .env file" << std::endl;
        exit(1);
    }

    CLIENT_ID = dotenv::get("CLIENT_ID");
    CLIENT_SECRET = dotenv::get("CLIENT_SECRET");
    BASE_URL = dotenv::get("BASE_URL");
}

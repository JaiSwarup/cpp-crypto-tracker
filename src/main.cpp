// main.cpp
#include "websocket_manager.hpp"
#include "config.h"
#include <iostream>
#include <csignal>
#include <atomic>

std::atomic<bool> running{true};
WebSocketManager* manager_ptr;

void signal_handler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
    running = false;
    if (manager_ptr) {
        manager_ptr->stop();
    }
}

int main() {
    // Register signal handler
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    loadConfig();

    const uint16_t port = 9002;

    try {
        DeribitClient deribit_client;
        WebSocketManager manager(deribit_client);
        manager_ptr = &manager;

        std::cout << "Starting WebSocket server on port " << port << std::endl;
        manager.run(port);
        // manager.stop();

        while (running && manager.is_running()) {
            std::cout << "Main thread running..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Program terminated successfully" << std::endl;
    return 0;
}
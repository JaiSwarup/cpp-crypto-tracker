#include "websocket_manager.hpp"
#include "order_manager.hpp"
#include "config.h"
#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

std::atomic<bool> running{true};
std::mutex mtx;
std::condition_variable cv;
WebSocketServer* server_ptr = nullptr;

void signal_handler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
    {
        std::lock_guard<std::mutex> lock(mtx);
        running = false;
        if (server_ptr) {
            server_ptr->stop(); 
        }
    }
    cv.notify_all(); 
}

int main() {
    loadConfig();
    

    const uint16_t port = 9002;

    try {
        DeribitClient deribit_client;
        deribit_client.authenticate();
        WebSocketServer server(deribit_client);
        OrderManager manager(deribit_client);
        server_ptr = &server;

        while (true) {
            {
                std::lock_guard<std::mutex> lock(mtx);
                if (!running) break;
            }
            std::cout << "==================================================================" << std::endl;
            std::cout << "DERIBIT ORDER MANAGEMENT PLATFORM" << std::endl;
            std::cout << "1. View Current Positions" << std::endl;
            std::cout << "2. Place Order" << std::endl;
            std::cout << "3. Cancel Order" << std::endl;
            std::cout << "4. Modify Order" << std::endl;
            std::cout << "5. View Orderbook" << std::endl;
            std::cout << "6. Start WebSocket Server" << std::endl;
            std::cout << "7. Exit" << std::endl;
            std::cout << "Enter your choice:  ";
            int choice;
            if (!(std::cin >> choice)) {
                //Handle input in case of SIGINT
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            switch (choice) {
                case 1: {
                    std::string currency, kind;
                    std::cout << "Enter currency: ";
                    std::cin >> currency;
                    std::cout << "Enter kind: ";
                    std::cin >> kind;
                    std::cout << manager.view_current_positions(currency, kind) << std::endl;
                    break;
                }
                case 2: {
                    std::string symbol, side, type, quantity, price;
                    std::cout << "Enter symbol: ";
                    std::cin >> symbol;
                    std::cout << "Enter side: ";
                    std::cin >> side;
                    std::cout << "Enter type: ";
                    std::cin >> type;
                    std::cout << "Enter quantity: ";
                    std::cin >> quantity;
                    std::cout << "Enter price: ";
                    std::cin >> price;
                    std::cout << manager.place_order(symbol, side, type, quantity, price) << std::endl;
                    break;
                }
                case 3: {
                    std::string order_id;
                    std::cout << "Enter order id: ";
                    std::cin >> order_id;
                    std::cout << manager.cancel_order(order_id) << std::endl;
                    break;
                }
                case 4: {
                    std::string order_id, quantity, price;
                    std::cout << "Enter order id: ";
                    std::cin >> order_id;
                    std::cout << "Enter quantity: ";
                    std::cin >> quantity;
                    std::cout << "Enter price: ";
                    std::cin >> price;
                    std::cout << manager.modify_order(order_id, quantity, price) << std::endl;
                    break;
                }
                case 5: {
                    std::string instrument_name;
                    std::cout << "Enter Instrument Name: ";
                    std::cin >> instrument_name;
                    std::cout << manager.get_orderbook(instrument_name) << std::endl;
                    break;
                }
                case 6: {
                    std::cout << "Starting WebSocket server on port " << port << std::endl;
                    std::thread server_thread([&]() {
                        server.run(port);
                    });
                    {
                        std::unique_lock<std::mutex> lock(mtx);
                        cv.wait(lock, [] { return !running; });
                    }
                    if (server.is_running()) {
                        server.stop();
                    }

                    if (server_thread.joinable()) {
                        server_thread.join();
                    }
                    break;
                }
                case 7: {
                    signal_handler(SIGINT);
                    break;
                }
                default: {
                    std::cout << "Invalid choice" << std::endl;
                }
            }
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (!running) break;
        }
        }
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Program terminated successfully" << std::endl;
    return 0;
}

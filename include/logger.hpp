#ifndef LOGGER_HPP
#define LOGGER_HPP

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

#include <iostream>
#include <ctime>
#include <iomanip>

class Logger{
public:
    enum class LogLevel {
        INFO,
        SUCCESS,
        WARNING,
        ERROR
    };

    void log(LogLevel level, const std::string& message) {
        std::time_t t = std::time(0);
        std::tm* now = std::localtime(&t);
        std::cout << "[" << std::put_time(now, "%F %T") << "] ";
        switch (level) {
            case LogLevel::INFO:
                std::cout << WHITE << "[INFO] " << message << RESET << std::endl;
                break;
            case LogLevel::WARNING:
                std::cout << YELLOW << "[WARNING] "<< message << RESET << std::endl;
                break;
            case LogLevel::ERROR:
                std::cout << RED << "[ERROR] "<< message << RESET << std::endl;
                break;
            case LogLevel::SUCCESS:
                std::cout << GREEN << "[SUCCESS] "<< message << RESET << std::endl;
                break;
        }
    }
};

#endif
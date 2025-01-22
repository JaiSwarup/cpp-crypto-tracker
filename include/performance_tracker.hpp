#ifndef PERFORMANCE_TRACKER_HPP
#define PERFORMANCE_TRACKER_HPP

#include <chrono>
#include <string>
#include <iostream>

class PerformanceTracker {
public:
    PerformanceTracker(const std::string& name): m_name(name) {
        start();
    }


    ~PerformanceTracker(){
    if (m_end_time == std::chrono::high_resolution_clock::time_point{}) {
        stop();
    }
}
    void start(){
    m_start_time = std::chrono::high_resolution_clock::now();
}
    void stop(){
    m_end_time = std::chrono::high_resolution_clock::now();
    std::cout << "Function " << m_name << " took "  << elapsed_time() << "ms" << std::endl;
}
    double elapsed_time() const{
    auto end = (m_end_time == std::chrono::high_resolution_clock::time_point{}) 
        ? std::chrono::high_resolution_clock::now() 
        : m_end_time;
    return std::chrono::duration<double, std::milli>(end - m_start_time).count();
}
private:
    std::string m_name;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_end_time;
};
#endif

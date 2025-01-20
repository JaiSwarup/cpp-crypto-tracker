cmake_minimum_required (VERSION 3.11)
project(deribit_c++ LANGUAGES CXX VERSION 1.0.0)

include_directories(${PROJECT_SOURCE_DIR}/include) # Your project's include directory
include_directories(${PROJECT_SOURCE_DIR}/websocketpp/websocketpp)

find_package(cpr REQUIRED)
find_package(ftxui 5 REQUIRED)
find_package(nlohmann_json 3 REQUIRED)

file(GLOB SOURCES "src/*.cpp")

add_executable(deribit_order_management ${SOURCES})
target_link_libraries(deribit_order_management
  PRIVATE
    cpr::cpr
    ftxui::screen
    ftxui::dom
    ftxui::component
    nlohmann_json::nlohmann_json
)

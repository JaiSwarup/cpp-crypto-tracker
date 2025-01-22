cmake_minimum_required (VERSION 3.11)
project(deribit_cpp LANGUAGES CXX VERSION 1.0.0)

include_directories(${PROJECT_SOURCE_DIR}/include) # Your project's include directory
include_directories(${PROJECT_SOURCE_DIR}/websocketpp)
include_directories(${Boost_INCLUDE_DIRS}) 

find_package(cpr REQUIRED)
find_package(ftxui 5 REQUIRED)
find_package(nlohmann_json 3 REQUIRED)
find_package(websocketpp REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)

file(GLOB SOURCES "src/*.cpp")

add_executable(deribit_cpp
  ${SOURCES}
)
target_link_libraries(deribit_cpp
  PRIVATE
    cpr::cpr
    websocketpp::websocketpp 
    OpenSSL::SSL
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
)

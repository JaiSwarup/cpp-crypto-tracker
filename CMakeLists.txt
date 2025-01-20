cmake_minimum_required (VERSION 3.11)
project(deribit_c++ LANGUAGES CXX VERSION 1.0.0)
include_directories(${PROJECT_SOURCE_DIR}/include) # Your project's include directory
include_directories(${Boost_INCLUDE_DIRS})         # Boost headers
find_package(cpr REQUIRED)
find_package(ftxui 5 REQUIRED)
add_executable(main
src/main.cpp
src/config.cpp
src/dotenv.cpp
src/deribit_client.cpp
)
target_link_libraries(main
  PRIVATE
    cpr::cpr
    ftxui::screen
    ftxui::dom
    ftxui::component
)
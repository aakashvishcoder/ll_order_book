# CMakeLists.txt
cmake_minimum_required(VERSION 3.14)
project(LowLatencyOrderBook VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find threading library
find_package(Threads REQUIRED)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Core Library
add_library(OrderBookLib STATIC
    src/OrderBook.cpp
    src/MatchingEngine.cpp
)
target_link_libraries(OrderBookLib PUBLIC Threads::Threads)

# Main Executable
add_executable(orderbook_app src/main.cpp)
target_link_libraries(orderbook_app PRIVATE OrderBookLib)

# Stress Test Executable
add_executable(stress_test tests/stress_test.cpp)
target_link_libraries(stress_test PRIVATE OrderBookLib)

# Compiler optimizations for Release
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(OrderBookLib PRIVATE -O3 -march=native)
endif()
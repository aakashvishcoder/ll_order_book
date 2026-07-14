# CMakeLists.txt
cmake_minimum_required(VERSION 3.14)
project(LowLatencyOrderBook VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find threading library
find_package(Threads REQUIRED)

# Include directories
include_directories(${PROJECT_SOURCE_DIR})

# Core Library
add_library(OrderBookLib STATIC
    src/OrderBook.cpp
)
target_link_libraries(OrderBookLib PUBLIC Threads::Threads)

# Stress Test Executable
add_executable(stress_test tests/stress_test.cpp)
target_link_libraries(stress_test PRIVATE OrderBookLib)

# Compiler optimizations for Release
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(OrderBookLib PRIVATE -O3 -march=native)
    elseif (MSVC)
        target_compile_options(OrderBookLib PRIVATE /O2)
    endif()
endif()

# Add Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(unit_tests tests/unit_tests.cpp)
target_link_libraries(unit_tests PRIVATE OrderBookLib GTest::gtest_main)
add_test(NAME OrderBookTests COMMAND unit_tests)

# Check for CUDA support
include(CheckLanguage)
check_language(CUDA)

if(CMAKE_CUDA_COMPILER)
    enable_language(CUDA)
    find_package(CUDAToolkit REQUIRED)
    
    set(CMAKE_CUDA_STANDARD 17)
    set(CMAKE_CUDA_STANDARD_REQUIRED ON)
    
    # Add the GPU Analytics Library
    add_library(GpuAnalyticsLib STATIC 
        src/GpuAnalytics.cpp 
        src/analytics_kernel.cu
    )
    set_target_properties(GpuAnalyticsLib PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
    target_link_libraries(GpuAnalyticsLib PRIVATE CUDA::cudart OrderBookLib)
    target_include_directories(GpuAnalyticsLib PUBLIC ${PROJECT_SOURCE_DIR}/include)
    
    # Link to main app
    target_link_libraries(orderbook_app PRIVATE GpuAnalyticsLib)
    target_link_libraries(stress_test PRIVATE GpuAnalyticsLib)
else()
    message(WARNING "CUDA compiler not found. GPU Analytics will be disabled.")
endif()
cmake_minimum_required(VERSION 3.10)
project(Week3_Homework VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Use position-independent code for shared libraries
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Paths to previous weeks' code for integration
set(WEEK1_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../Week1)
set(WEEK2_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../Week2)

# Add include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    ${WEEK1_DIR}/include      # Week 1 headers (sorting algorithms)
    ${WEEK2_DIR}/include      # Week 2 headers (memory management)
)

# Find required packages
find_package(Threads REQUIRED)
find_package(GTest REQUIRED)

# Add subdirectories for Week 1 and Week 2 code if needed
# Uncomment if you need to build components from previous weeks
# add_subdirectory(${WEEK1_DIR} week1_build)
# add_subdirectory(${WEEK2_DIR} week2_build)

# Create library with the implementations
add_library(trading_components STATIC
    src/impl/market_data_handler.cpp
    # Add your thread pool, lock-free queue implementations
    # Include any Week 1 and Week 2 implementation files needed
    ${WEEK2_DIR}/src/memory_management/order_book_allocator.cpp  # Example Week 2 file
)

# Link with thread library
target_link_libraries(trading_components
    PRIVATE Threads::Threads
)

# Add tests
enable_testing()

# Market data handler test
add_executable(market_data_handler_test
    tests/market_data_handler_test.cpp
)

target_link_libraries(market_data_handler_test
    PRIVATE trading_components
    PRIVATE GTest::GTest
    PRIVATE GTest::Main
    PRIVATE Threads::Threads
)

add_test(NAME MarketDataHandlerTest COMMAND market_data_handler_test)

# Thread pool test
add_executable(thread_pool_test
    tests/thread_pool_test.cpp
)

target_link_libraries(thread_pool_test
    PRIVATE trading_components
    PRIVATE GTest::GTest
    PRIVATE GTest::Main
    PRIVATE Threads::Threads
)

add_test(NAME ThreadPoolTest COMMAND thread_pool_test)

# Lock-free queue test
add_executable(lock_free_queue_test
    tests/lock_free_queue_test.cpp
)

target_link_libraries(lock_free_queue_test
    PRIVATE trading_components
    PRIVATE GTest::GTest
    PRIVATE GTest::Main
    PRIVATE Threads::Threads
)

add_test(NAME LockFreeQueueTest COMMAND lock_free_queue_test)

# Integration test (combines Week 1-3 components)
add_executable(integrated_system_test
    tests/integrated_system_test.cpp
)

target_link_libraries(integrated_system_test
    PRIVATE trading_components
    PRIVATE GTest::GTest
    PRIVATE GTest::Main
    PRIVATE Threads::Threads
)

add_test(NAME IntegratedSystemTest COMMAND integrated_system_test)

# Performance benchmarks
# Market data handler benchmark
add_executable(market_data_handler_perf
    benchmarks/market_data_handler_perf.cpp
)

target_link_libraries(market_data_handler_perf
    PRIVATE trading_components
    PRIVATE Threads::Threads
)

# Thread pool benchmark
add_executable(thread_pool_perf
    benchmarks/thread_pool_perf.cpp
)

target_link_libraries(thread_pool_perf
    PRIVATE trading_components
    PRIVATE Threads::Threads
)

# Lock-free queue benchmark
add_executable(lock_free_queue_perf
    benchmarks/lock_free_queue_perf.cpp
)

target_link_libraries(lock_free_queue_perf
    PRIVATE trading_components
    PRIVATE Threads::Threads
)

# Integrated performance test
add_executable(integrated_performance_test
    benchmarks/integrated_performance_test.cpp
)

target_link_libraries(integrated_performance_test
    PRIVATE trading_components
    PRIVATE Threads::Threads
)

# Add an empty market_data_handler.cpp file if it doesn't exist
file(WRITE ${PROJECT_SOURCE_DIR}/src/impl/market_data_handler.cpp
"#include \"../market_data_handler.hpp\"

// TODO: Implement the MarketDataHandler class
// Use your Week 2 OrderBookAllocator for memory management
// Use your Week 1 sorting algorithms for maintaining price levels

namespace trading {

// Constructor
MarketDataHandler::MarketDataHandler(size_t max_symbols) {
    // TODO: Initialize the handler
    // Use memory management from Week 2
}

// Destructor
MarketDataHandler::~MarketDataHandler() {
    // TODO: Clean up resources
}

// Add exchange
bool MarketDataHandler::add_exchange(const std::string& exchange_name) {
    // TODO: Implement this method
    return false;
}

// Subscribe to market data
bool MarketDataHandler::subscribe(const std::string& symbol, MarketDataCallback callback) {
    // TODO: Implement this method
    return false;
}

// Unsubscribe from market data
bool MarketDataHandler::unsubscribe(const std::string& symbol) {
    // TODO: Implement this method
    return false;
}

// Process market update
void MarketDataHandler::process_update(const MarketUpdate& update) {
    // TODO: Implement this method
    // Ensure thread safety and efficient memory usage
}

// Get order book
OrderBook MarketDataHandler::get_order_book(const std::string& symbol) const {
    // TODO: Implement this method
    // Use Week 1 sorting for returning sorted price levels
    return OrderBook{};
}

// Get metrics
MarketDataMetrics MarketDataHandler::get_metrics() const {
    // TODO: Implement this method
    return MarketDataMetrics{};
}

// Start processing
void MarketDataHandler::start() {
    // TODO: Implement this method
    // Start the processing threads
}

// Stop processing
void MarketDataHandler::stop() {
    // TODO: Implement this method
    // Safely stop all threads
}

// Exchange thread function
void MarketDataHandler::exchange_thread_func(const std::string& exchange_name) {
    // TODO: Implement this method
    // This is where the thread processes exchange data
}

// Update metrics
void MarketDataHandler::update_metrics(const std::string& exchange, 
                                      std::chrono::nanoseconds processing_time) {
    // TODO: Implement this method
    // Update the performance metrics
}

} // namespace trading
")

# Create empty test files if they don't exist
file(WRITE ${PROJECT_SOURCE_DIR}/tests/integrated_system_test.cpp
"#include <gtest/gtest.h>
#include \"market_data_handler.hpp\"
// Include headers from Week 1 (sorting) and Week 2 (memory management)

namespace {

TEST(IntegratedSystemTest, CombinesAllComponents) {
    // This test should validate that Week 1-3 components work together correctly
    
    // TODO: Setup test
    // 1. Initialize Week 2 memory allocators
    // 2. Create market data handler using the allocators
    // 3. Use Week 1 sorting algorithms where needed
    // 4. Verify the integrated system works correctly
    
    EXPECT_TRUE(true); // Replace with actual test conditions
}

} // namespace
")

file(WRITE ${PROJECT_SOURCE_DIR}/benchmarks/integrated_performance_test.cpp
"#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>
#include \"market_data_handler.hpp\"
#include \"thread_pool.hpp\"
#include \"lock_free_queue.hpp\"
// Include headers from Week 1 (sorting) and Week 2 (memory management)

using namespace std;
using namespace trading;
using namespace std::chrono;

// This benchmark tests the performance of the integrated system
// combining components from Weeks 1-3

int main() {
    cout << \"===== Integrated System Performance Test =====\\n\";
    
    // TODO: Setup the integrated benchmark
    // 1. Initialize Week 2 memory allocators
    // 2. Create threads and data structures
    // 3. Measure end-to-end performance
    
    cout << \"Test completed!\\n\";
    return 0;
}
")

# Create empty directories if they don't exist
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/src/impl)
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/benchmarks)
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/tests) 
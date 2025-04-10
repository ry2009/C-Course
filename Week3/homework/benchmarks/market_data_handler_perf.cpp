#include "../src/market_data_handler.hpp"
// Include Week 1 sorting headers
// For example: #include "../../../Week1/include/sorting/quick_sort.hpp"
// Include Week 2 memory management headers
// For example: #include "../../../Week2/include/memory/order_book_allocator.hpp"

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <atomic>
#include <iomanip>
#include <random>

using namespace trading;
using namespace std::chrono;

// Sample market data generator for benchmark
class MarketDataGenerator {
private:
    std::mt19937 rng;
    std::uniform_real_distribution<double> price_dist;
    std::uniform_int_distribution<uint64_t> volume_dist;
    std::vector<std::string> symbols;
    std::vector<std::string> exchanges;

public:
    MarketDataGenerator() 
        : rng(std::random_device{}()), 
          price_dist(1.0, 1000.0),
          volume_dist(1, 1000) {
        // Initialize symbols and exchanges
        symbols = {"AAPL", "MSFT", "GOOGL", "AMZN", "TSLA", "FB", "BRK.A", "V", "JPM", "JNJ"};
        exchanges = {"NYSE", "NASDAQ", "LSE"};
    }

    // Generate a random market update
    MarketUpdate generate_update() {
        MarketUpdate update;
        update.symbol = symbols[rng() % symbols.size()];
        update.exchange = exchanges[rng() % exchanges.size()];
        update.bid_price = price_dist(rng);
        update.ask_price = update.bid_price + 0.01 + price_dist(rng) * 0.01;
        update.last_price = (update.bid_price + update.ask_price) / 2.0;
        update.volume = volume_dist(rng);
        update.timestamp = high_resolution_clock::now().time_since_epoch();
        return update;
    }

    // Get available symbols
    const std::vector<std::string>& get_symbols() const {
        return symbols;
    }

    // Get available exchanges
    const std::vector<std::string>& get_exchanges() const {
        return exchanges;
    }
};

// Simple callback to track updates
void market_data_callback(const MarketUpdate& update) {
    // In a real test, we might track statistics here
    // But for benchmark we keep it minimal
}

int main() {
    std::cout << "===== Market Data Handler Performance Test =====\n";
    std::cout << "This test measures the performance of the MarketDataHandler\n";
    std::cout << "using Week 2's memory management and Week 1's sorting algorithms\n\n";

    const size_t MAX_SYMBOLS = 100;
    const size_t UPDATES_PER_EXCHANGE = 100000;
    MarketDataGenerator generator;

    // Create and start the market data handler
    // Using Week 2's memory management for allocation
    MarketDataHandler handler(MAX_SYMBOLS);

    // Add exchanges (which creates threads for each)
    for (const auto& exchange : generator.get_exchanges()) {
        handler.add_exchange(exchange);
    }

    // Subscribe to market data for all symbols
    // This will use your Week 1 sorting to keep the price levels sorted
    for (const auto& symbol : generator.get_symbols()) {
        handler.subscribe(symbol, market_data_callback);
    }

    // Start the handler threads
    handler.start();

    // Wait for the handler to initialize
    std::this_thread::sleep_for(milliseconds(100));

    // Create a thread for each exchange to push updates
    std::vector<std::thread> exchange_threads;
    std::atomic<size_t> updates_processed(0);

    auto start_time = high_resolution_clock::now();

    // Create threads for each exchange to push market data
    for (const auto& exchange : generator.get_exchanges()) {
        exchange_threads.emplace_back([&handler, &generator, &updates_processed, 
                                       exchange, UPDATES_PER_EXCHANGE]() {
            for (size_t i = 0; i < UPDATES_PER_EXCHANGE; ++i) {
                auto update = generator.generate_update();
                update.exchange = exchange; // Force the correct exchange
                handler.process_update(update);
                updates_processed.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    // Wait until all updates are generated
    size_t total_updates = generator.get_exchanges().size() * UPDATES_PER_EXCHANGE;
    while (updates_processed.load(std::memory_order_relaxed) < total_updates) {
        std::this_thread::sleep_for(milliseconds(10));
        
        // Print progress
        auto current = updates_processed.load(std::memory_order_relaxed);
        std::cout << "\rProgress: " << current << "/" << total_updates 
                  << " (" << (current * 100 / total_updates) << "%)" << std::flush;
    }
    std::cout << std::endl;

    // Wait for threads to finish
    for (auto& thread : exchange_threads) {
        thread.join();
    }

    // Stop the handler
    handler.stop();

    // Calculate elapsed time
    auto end_time = high_resolution_clock::now();
    auto elapsed = duration_cast<milliseconds>(end_time - start_time);

    // Print results
    std::cout << "\nResults:\n";
    std::cout << "-------------------------------\n";
    std::cout << "Total updates processed: " << total_updates << std::endl;
    std::cout << "Total time: " << elapsed.count() << " ms" << std::endl;
    std::cout << "Throughput: " << (total_updates * 1000.0 / elapsed.count()) 
              << " updates/second" << std::endl;

    // Print performance metrics from the handler
    auto metrics = handler.get_metrics();
    std::cout << "\nHandler Metrics:\n";
    std::cout << "-------------------------------\n";
    
    // Print latency per exchange
    std::cout << "Average latency per exchange (microseconds):\n";
    for (const auto& [exchange, latency] : metrics.avg_latency_us) {
        std::cout << "  " << exchange << ": " << latency << " µs\n";
    }
    
    // Print throughput per exchange
    std::cout << "\nThroughput per exchange (messages per second):\n";
    for (const auto& [exchange, throughput] : metrics.throughput_mps) {
        std::cout << "  " << exchange << ": " << throughput << " msg/s\n";
    }
    
    // Print lock contention statistics
    std::cout << "\nLock contentions: " << metrics.lock_contentions.load() << std::endl;
    std::cout << "Lock wait time: " << metrics.lock_wait_time_ns.load() / 1000000.0 << " ms" << std::endl;
    
    // Print overall statistics
    std::cout << "\nTotal updates processed by handler: " << metrics.total_updates_processed.load() << std::endl;
    std::cout << "Total updates dropped: " << metrics.total_updates_dropped.load() << std::endl;
    
    std::cout << "\nPerformance test completed!\n";
    return 0;
} 
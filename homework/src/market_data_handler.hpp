#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <chrono>
#include <atomic>
#include <mutex>
#include <thread>
#include <memory>

namespace trading {

// Forward declarations
struct OrderBook;
struct MarketUpdate;

/**
 * @brief Performance metrics for the market data handler
 */
struct MarketDataMetrics {
    // Average processing latency in microseconds per exchange
    std::unordered_map<std::string, double> avg_latency_us;
    
    // Messages per second per exchange
    std::unordered_map<std::string, double> throughput_mps;
    
    // Lock contention statistics
    std::atomic<uint64_t> lock_contentions{0};
    std::atomic<uint64_t> lock_wait_time_ns{0};
    
    // Aggregate statistics
    std::atomic<uint64_t> total_updates_processed{0};
    std::atomic<uint64_t> total_updates_dropped{0};
};

/**
 * @brief Callback type for market data updates
 */
using MarketDataCallback = std::function<void(const MarketUpdate&)>;

/**
 * @brief Market update structure containing latest prices
 */
struct MarketUpdate {
    std::string symbol;       // Ticker symbol
    std::string exchange;     // Source exchange
    double bid_price;         // Best bid price
    double ask_price;         // Best ask price
    double last_price;        // Last trade price
    uint64_t volume;          // Volume
    std::chrono::nanoseconds timestamp; // Update timestamp
};

/**
 * @brief Order book entry representing a price level
 */
struct OrderBookEntry {
    double price;
    uint64_t size;
};

/**
 * @brief Order book structure for a single instrument
 */
struct OrderBook {
    std::string symbol;
    std::vector<OrderBookEntry> bids;  // Sorted by price (descending)
    std::vector<OrderBookEntry> asks;  // Sorted by price (ascending)
    std::chrono::nanoseconds timestamp;
    
    // Add any additional fields you need
};

/**
 * @brief Thread-safe market data handler
 * 
 * This class is responsible for processing market data from multiple exchanges
 * and maintaining up-to-date order books. It must be thread-safe to handle
 * concurrent updates from different exchange threads.
 */
class MarketDataHandler {
public:
    /**
     * @brief Construct a new Market Data Handler
     * 
     * @param max_symbols Maximum number of symbols to track
     */
    explicit MarketDataHandler(size_t max_symbols);
    
    /**
     * @brief Destructor - ensures all exchange threads are stopped cleanly
     */
    ~MarketDataHandler();
    
    /**
     * @brief Add a new exchange source
     * 
     * @param exchange_name Name of the exchange
     * @return true if added successfully, false otherwise
     */
    bool add_exchange(const std::string& exchange_name);
    
    /**
     * @brief Subscribe to market data for a symbol
     * 
     * @param symbol The ticker symbol to subscribe to
     * @param callback Callback function to receive updates
     * @return true if subscription successful, false otherwise
     */
    bool subscribe(const std::string& symbol, MarketDataCallback callback);
    
    /**
     * @brief Unsubscribe from market data for a symbol
     * 
     * @param symbol The ticker symbol to unsubscribe from
     * @return true if unsubscription successful, false otherwise
     */
    bool unsubscribe(const std::string& symbol);
    
    /**
     * @brief Process a market update from any exchange
     * 
     * This method must be thread-safe as it will be called concurrently
     * from multiple exchange threads.
     * 
     * @param update The market update to process
     */
    void process_update(const MarketUpdate& update);
    
    /**
     * @brief Get the current order book for a symbol
     * 
     * @param symbol The ticker symbol
     * @return A copy of the current order book
     */
    OrderBook get_order_book(const std::string& symbol) const;
    
    /**
     * @brief Get performance metrics for the market data handler
     * 
     * @return Current performance metrics
     */
    MarketDataMetrics get_metrics() const;
    
    /**
     * @brief Start processing market data
     * 
     * Starts all exchange threads and begins processing market data.
     */
    void start();
    
    /**
     * @brief Stop processing market data
     * 
     * Stops all exchange threads and shuts down processing.
     */
    void stop();

private:
    // TODO: Implement private member variables and methods
    
    // Suggestion: You'll need thread-safe data structures for:
    // 1. Order books for each symbol
    // 2. Subscription information
    // 3. Exchange thread management
    // 4. Performance metrics tracking
    
    // Mock exchange thread function - simulates receiving market data
    void exchange_thread_func(const std::string& exchange_name);
    
    // Helper method to track and update performance metrics
    void update_metrics(const std::string& exchange, 
                        std::chrono::nanoseconds processing_time);
};

} // namespace trading 
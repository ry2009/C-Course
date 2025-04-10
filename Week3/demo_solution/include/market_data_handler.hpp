#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <shared_mutex> // Week 3 optimization: Reader-writer lock
#include <atomic>      // Week 3 optimization: Lock-free counters
#include <functional>
#include <chrono>
#include <thread>
#include <memory>
#include "order_book_allocator.hpp"

/**
 * @file market_data_handler.hpp
 * @brief Market data handler implementation that integrates optimizations from Weeks 1-3.
 * 
 * This file demonstrates integration of:
 * - Week 1: Optimized sorting algorithms for order book maintenance
 * - Week 2: Custom memory allocators for efficient order book memory management
 * - Week 3: Thread-safe components using reader-writer locks and atomic operations
 */

namespace trading {

// Forward declaration of Week2 OrderBookAllocator
namespace week2 {
    class OrderBookAllocator;
}

/**
 * @brief Structure representing an entry in the order book.
 * 
 * Using a simple struct for cache-friendly memory layout.
 * This is an optimization for high-frequency trading systems.
 */
struct OrderBookEntry {
    double price;
    int volume;
    
    // Needed for sorting
    bool operator<(const OrderBookEntry& other) const {
        return price < other.price;
    }
    
    bool operator>(const OrderBookEntry& other) const {
        return price > other.price;
    }
};

/**
 * @brief Structure representing an order book for a financial instrument.
 * 
 * Maintained with sorting algorithms from Week 1 and memory management from Week 2.
 * The bids and asks are separate vectors to allow for efficient processing of
 * each side of the book independently.
 */
struct OrderBook {
    std::string symbol;
    std::chrono::nanoseconds timestamp;
    std::vector<OrderBookEntry> bids; // Sorted by price (descending)
    std::vector<OrderBookEntry> asks; // Sorted by price (ascending)
};

/**
 * @brief Structure representing a market update from an exchange.
 * 
 * Contains all the necessary information for updating an order book and
 * calculating trading signals. The timestamp is in nanoseconds to support
 * high-frequency trading applications.
 */
struct MarketUpdate {
    std::string symbol;
    std::string exchange;
    double bid_price;
    double ask_price;
    int volume;
    std::chrono::nanoseconds timestamp;
};

/**
 * @brief Structure containing metrics about market data processing.
 * 
 * Uses atomic variables for thread-safe access without locking.
 * This is a key Week 3 optimization for maintaining metrics without
 * affecting the critical path of market data processing.
 */
struct MarketDataMetrics {
    // Default constructor
    MarketDataMetrics() = default;
    
    // Delete copy constructor and assignment operator to prevent copying atomic members
    MarketDataMetrics(const MarketDataMetrics&) = delete;
    MarketDataMetrics& operator=(const MarketDataMetrics&) = delete;
    
    // Atomic counters for lock-free updates - Week 3 optimization
    std::atomic<uint64_t> total_updates_processed{0};
    std::atomic<uint64_t> total_updates_dropped{0};
    std::atomic<uint64_t> lock_contentions{0};
    std::atomic<uint64_t> lock_wait_time_ns{0};
    
    // Maps require mutex protection since they're not atomic
    std::unordered_map<std::string, double> avg_latency_us;   // Average latency in microseconds by exchange
    std::unordered_map<std::string, double> throughput_mps;   // Throughput in messages per second by exchange
};

/**
 * @brief Non-atomic version of metrics for returning from get_metrics().
 * 
 * This struct allows us to return metrics without atomic members,
 * addressing the copy issue with std::atomic.
 */
struct MarketDataMetricsResult {
    uint64_t total_updates_processed{0};
    uint64_t total_updates_dropped{0};
    uint64_t lock_contentions{0};
    uint64_t lock_wait_time_ns{0};
    std::unordered_map<std::string, double> avg_latency_us;
    std::unordered_map<std::string, double> throughput_mps;
};

// Type for market data callback
using MarketDataCallback = std::function<void(const MarketUpdate&)>;

/**
 * @brief Thread-safe market data handler implementation.
 * 
 * This class integrates optimizations from all three weeks:
 * - Week 1: Uses optimized sorting algorithms for order book maintenance
 * - Week 2: Uses custom allocators for efficient memory management
 * - Week 3: Implements thread-safe components with multiple optimization strategies:
 *   - Reader-writer locks for concurrent read access
 *   - Fine-grained locking to minimize contention
 *   - Lock-free metrics with atomic variables
 *   - Per-exchange threading for parallel processing
 */
class MarketDataHandler {
public:
    /**
     * @brief Constructs a new Market Data Handler.
     * 
     * @param max_symbols Maximum number of symbols to support (for pre-allocation)
     */
    explicit MarketDataHandler(size_t max_symbols);
    
    /**
     * @brief Destroys the Market Data Handler, cleaning up resources.
     */
    ~MarketDataHandler();
    
    /**
     * @brief Add an exchange to the handler.
     * 
     * Creates a dedicated thread for processing exchange data.
     * Thread-safe operation protected by exchanges_mutex_.
     * 
     * @param exchange_name Name of the exchange
     * @return true if added successfully, false if already exists
     */
    bool add_exchange(const std::string& exchange_name);
    
    /**
     * @brief Subscribe to market data for a symbol.
     * 
     * Registers a callback for market updates for the specified symbol.
     * Thread-safe operation protected by books_mutex_ (writer lock).
     * 
     * @param symbol Symbol to subscribe to
     * @param callback Callback function to call on updates
     * @return true if subscribed successfully, false if at capacity
     */
    bool subscribe(const std::string& symbol, MarketDataCallback callback);
    
    /**
     * @brief Unsubscribe from market data for a symbol.
     * 
     * Thread-safe operation protected by books_mutex_ (writer lock).
     * 
     * @param symbol Symbol to unsubscribe from
     * @return true if unsubscribed successfully, false if not found
     */
    bool unsubscribe(const std::string& symbol);
    
    /**
     * @brief Process a market update.
     * 
     * Updates the order book and calls the registered callback.
     * Uses a reader-writer lock pattern for efficient concurrent access.
     * Implements the Week 1 sorting optimization for order book entries.
     * 
     * @param update Market update to process
     */
    void process_update(const MarketUpdate& update);
    
    /**
     * @brief Get the order book for a symbol.
     * 
     * Thread-safe operation protected by books_mutex_ (reader lock).
     * 
     * @param symbol Symbol to get order book for
     * @return OrderBook Current state of the order book
     */
    OrderBook get_order_book(const std::string& symbol) const;
    
    /**
     * @brief Get metrics about market data processing.
     * 
     * @return MarketDataMetricsResult Current metrics
     */
    MarketDataMetricsResult get_metrics() const;
    
    /**
     * @brief Start processing market data.
     * 
     * Starts a thread for each registered exchange.
     * Thread-safe operation protected by exchanges_mutex_.
     */
    void start();
    
    /**
     * @brief Stop processing market data.
     * 
     * Stops all exchange threads gracefully.
     * Thread-safe operation protected by exchanges_mutex_.
     */
    void stop();
    
private:
    // Thread function for exchange processing
    void exchange_thread_func(const std::string& exchange_name);
    
    // Update metrics with processing time
    void update_metrics(const std::string& exchange, std::chrono::nanoseconds processing_time);
    
    // Configuration
    size_t max_symbols_;
    
    // Exchange threads
    std::unordered_map<std::string, std::thread> exchange_threads_;
    std::mutex exchanges_mutex_;
    std::atomic<bool> running_;
    
    // Order books and callbacks
    mutable std::shared_mutex books_mutex_; // Week 3 optimization: Reader-writer lock
    std::unordered_map<std::string, OrderBook> order_books_;
    std::unordered_map<std::string, MarketDataCallback> callbacks_;
    
    // Metrics
    MarketDataMetrics metrics_;
    mutable std::mutex metrics_mutex_;
    
    // Week 2 memory management
    std::shared_ptr<week2::OrderBookAllocator> order_book_allocator_;
};

} // namespace trading 
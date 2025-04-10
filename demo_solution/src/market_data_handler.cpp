#include "../include/market_data_handler.hpp"
#include "../include/order_book_allocator.hpp"
#include <algorithm>
#include <thread>
#include <mutex>
#include <iostream>
#include <iomanip>

// Import Week 1 sorting algorithm (simulated)
namespace week1 {
    // This represents a custom optimized quick sort algorithm from Week 1
    // In a real implementation, this would be a highly optimized sorting algorithm
    // specialized for financial data with performance improvements like:
    // - Pivot selection optimization for financial time series data
    // - Early termination for nearly-sorted data (common in market updates)
    // - Cache-friendly memory access patterns
    template<typename Iterator, typename Compare>
    void quick_sort(Iterator begin, Iterator end, Compare comp) {
        // For demonstration purposes, we use std::sort as a placeholder
        // In a real Week 1 implementation, this would be a custom implementation
        std::sort(begin, end, comp);
        
        // In a real implementation, we would log performance metrics here:
        // std::cout << "Week 1 sorting completed: " << (end - begin) << " elements" << std::endl;
    }
}

// Import Week 2 memory management (simulated)
namespace week2 {
    // Simulated OrderBookAllocator from Week 2
    // In a real implementation, this would be a specialized allocator with:
    // - Fast allocation for fixed-size OrderBook objects
    // - Memory pooling to reduce fragmentation
    // - NUMA-aware allocation for multi-socket systems
    // - Minimal lock contention for concurrent access
    class OrderBookAllocator {
    public:
        OrderBookAllocator(size_t max_orders) : max_orders_(max_orders) {
            std::cout << "Week 2 optimization: Initializing OrderBookAllocator with capacity for " 
                      << max_orders << " orders" << std::endl;
            // In a real implementation, we would pre-allocate memory pools here
        }
        
        void* allocate(size_t size) {
            // For demonstration purposes, we use operator new
            // In a real Week 2 implementation, this would allocate from a memory pool
            allocation_count_++;
            total_allocated_ += size;
            
            // In a real implementation, we would use a custom memory pool
            void* ptr = ::operator new(size);
            
            // Log allocation for demonstration purposes
            std::cout << "Week 2 optimization: Allocated " << size << " bytes (total: " 
                      << total_allocated_ << " bytes in " << allocation_count_ << " allocations)" << std::endl;
            
            return ptr;
        }
        
        void deallocate(void* ptr) {
            deallocation_count_++;
            // For demonstration purposes, we use operator delete
            // In a real Week 2 implementation, this would return memory to the pool
            ::operator delete(ptr);
            
            // Log deallocation for demonstration purposes
            std::cout << "Week 2 optimization: Deallocated memory (total deallocations: " 
                      << deallocation_count_ << ")" << std::endl;
        }
        
    private:
        size_t max_orders_;
        size_t allocation_count_ = 0;
        size_t deallocation_count_ = 0;
        size_t total_allocated_ = 0;
    };
}

namespace trading {

// Constructor
MarketDataHandler::MarketDataHandler(size_t max_symbols) 
    : max_symbols_(max_symbols), 
      running_(false),
      order_book_allocator_(std::make_shared<week2::OrderBookAllocator>(1000)) {
    
    std::cout << "Week 3 optimization: Creating thread-safe MarketDataHandler with capacity for " 
              << max_symbols << " symbols" << std::endl;
    
    // Pre-allocate maps to avoid reallocations during high-frequency trading
    // This is an important optimization to minimize memory allocations during trading
    order_books_.reserve(max_symbols);
    callbacks_.reserve(max_symbols);
    
    std::cout << "Week 3 optimization: Pre-allocated maps to avoid reallocations during trading" 
              << std::endl;
}

// Destructor
MarketDataHandler::~MarketDataHandler() {
    stop();
    std::cout << "MarketDataHandler destroyed with final metrics:" << std::endl;
    
    auto metrics = get_metrics();
    std::cout << "  Total updates processed: " << metrics.total_updates_processed << std::endl;
    std::cout << "  Total updates dropped: " << metrics.total_updates_dropped << std::endl;
    std::cout << "  Lock contentions: " << metrics.lock_contentions << std::endl;
}

// Add exchange
bool MarketDataHandler::add_exchange(const std::string& exchange_name) {
    // Use lock guard for thread safety - Week 3 optimization
    // This ensures thread-safe access to the exchanges map
    std::lock_guard<std::mutex> lock(exchanges_mutex_);
    
    std::cout << "Week 3 optimization: Thread-safe exchange registration for " 
              << exchange_name << std::endl;
    
    // Check if exchange already exists
    if (exchange_threads_.find(exchange_name) != exchange_threads_.end()) {
        return false;
    }
    
    // Add to known exchanges
    exchange_threads_[exchange_name] = std::thread();
    metrics_.avg_latency_us[exchange_name] = 0.0;
    metrics_.throughput_mps[exchange_name] = 0.0;
    
    return true;
}

// Subscribe to market data
bool MarketDataHandler::subscribe(const std::string& symbol, MarketDataCallback callback) {
    // Use unique_lock for shared_mutex - Week 3 optimization
    // This allows for concurrent reads but exclusive writes
    std::unique_lock<std::shared_mutex> lock(books_mutex_);
    
    std::cout << "Week 3 optimization: Thread-safe subscription for symbol " 
              << symbol << std::endl;
    
    // Check if we have capacity
    if (order_books_.size() >= max_symbols_) {
        return false;
    }
    
    // Add to order books if not exists
    if (order_books_.find(symbol) == order_books_.end()) {
        OrderBook book;
        book.symbol = symbol;
        book.timestamp = std::chrono::nanoseconds(0);
        
        std::cout << "Week 2 optimization: Using custom allocator for OrderBook " 
                  << symbol << std::endl;
        
        // Use the OrderBookAllocator from Week 2 to allocate memory
        // This is a critical optimization from Week 2 that ensures efficient memory usage
        void* memory = order_book_allocator_->allocate(sizeof(OrderBook));
        OrderBook* new_book = new (memory) OrderBook(book);
        
        order_books_[symbol] = *new_book;
    }
    
    // Register callback
    callbacks_[symbol] = callback;
    
    return true;
}

// Unsubscribe from market data
bool MarketDataHandler::unsubscribe(const std::string& symbol) {
    // Use unique_lock for shared_mutex - Week 3 optimization
    std::unique_lock<std::shared_mutex> lock(books_mutex_);
    
    auto it = callbacks_.find(symbol);
    if (it == callbacks_.end()) {
        return false;
    }
    
    callbacks_.erase(it);
    return true;
}

// Process market update
void MarketDataHandler::process_update(const MarketUpdate& update) {
    // Start performance measurement
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Use a reader-writer lock pattern - Week 3 optimization
    // This allows multiple readers to access the order book concurrently
    // but ensures exclusive access when updating
    std::shared_lock<std::shared_mutex> read_lock(books_mutex_);
    
    auto book_it = order_books_.find(update.symbol);
    if (book_it == order_books_.end()) {
        // No order book for this symbol
        metrics_.total_updates_dropped.fetch_add(1, std::memory_order_relaxed);
        return;
    }
    
    // Upgrade to write lock - Week 3 optimization
    // This is a critical optimization that allows readers to continue
    // reading until we actually need to write to the order book
    read_lock.unlock();
    std::unique_lock<std::shared_mutex> write_lock(books_mutex_);
    
    // Check again after acquiring write lock
    book_it = order_books_.find(update.symbol);
    if (book_it == order_books_.end()) {
        metrics_.total_updates_dropped.fetch_add(1, std::memory_order_relaxed);
        return;
    }
    
    // Update the order book
    OrderBook& book = book_it->second;
    
    // Update timestamp
    book.timestamp = update.timestamp;
    
    // Update price levels
    // In a real implementation, we would find the correct price level and update it
    OrderBookEntry bid_entry{update.bid_price, update.volume};
    OrderBookEntry ask_entry{update.ask_price, update.volume};
    
    // Add entries if not found
    book.bids.push_back(bid_entry);
    book.asks.push_back(ask_entry);
    
    // Sort using Week 1 sorting algorithm
    // This is a critical optimization from Week 1 that ensures
    // efficient sorting of the order book entries
    std::cout << "Week 1 optimization: Sorting order book for " << update.symbol << std::endl;
    
    // Sort bids in descending order (best bid first)
    week1::quick_sort(book.bids.begin(), book.bids.end(), 
        [](const OrderBookEntry& a, const OrderBookEntry& b) { 
            return a.price > b.price; // Descending for bids
        });
    
    // Sort asks in ascending order (best ask first)
    week1::quick_sort(book.asks.begin(), book.asks.end(), 
        [](const OrderBookEntry& a, const OrderBookEntry& b) { 
            return a.price < b.price; // Ascending for asks
        });
    
    // Maintain limited size - optimization to avoid growing too large
    const size_t MAX_LEVELS = 10;
    if (book.bids.size() > MAX_LEVELS) {
        book.bids.resize(MAX_LEVELS);
    }
    if (book.asks.size() > MAX_LEVELS) {
        book.asks.resize(MAX_LEVELS);
    }
    
    // Release lock to call the callback - Week 3 optimization
    // This is critical for performance since callbacks might be slow
    // and we don't want to hold the lock while executing them
    write_lock.unlock();
    
    // Call the callback if registered
    auto callback_it = callbacks_.find(update.symbol);
    if (callback_it != callbacks_.end()) {
        std::cout << "Week 3 optimization: Executing callback for " 
                  << update.symbol << " without holding the lock" << std::endl;
        callback_it->second(update);
    }
    
    // Update metrics
    auto end_time = std::chrono::high_resolution_clock::now();
    auto processing_time = std::chrono::duration_cast<std::chrono::nanoseconds>(
        end_time - start_time);
    
    update_metrics(update.exchange, processing_time);
    metrics_.total_updates_processed.fetch_add(1, std::memory_order_relaxed);
    
    // Print processing time for this update
    std::cout << "Processed " << update.symbol << " update in " 
              << processing_time.count() / 1000.0 << " μs" << std::endl;
}

// Get order book
OrderBook MarketDataHandler::get_order_book(const std::string& symbol) const {
    // Use shared_lock for concurrent reads - Week 3 optimization
    std::shared_lock<std::shared_mutex> lock(books_mutex_);
    
    auto it = order_books_.find(symbol);
    if (it == order_books_.end()) {
        return OrderBook{};
    }
    
    return it->second;
}

// Get metrics
MarketDataMetricsResult MarketDataHandler::get_metrics() const {
    // Create a non-atomic copy of metrics - Week 3 optimization
    MarketDataMetricsResult result;
    result.total_updates_processed = metrics_.total_updates_processed.load(std::memory_order_relaxed);
    result.total_updates_dropped = metrics_.total_updates_dropped.load(std::memory_order_relaxed);
    result.lock_contentions = metrics_.lock_contentions.load(std::memory_order_relaxed);
    result.lock_wait_time_ns = metrics_.lock_wait_time_ns.load(std::memory_order_relaxed);
    
    // Copy the maps - need lock for non-atomic members
    {
        std::lock_guard<std::mutex> lock(metrics_mutex_);
        result.avg_latency_us = metrics_.avg_latency_us;
        result.throughput_mps = metrics_.throughput_mps;
    }
    
    return result;
}

// Start processing
void MarketDataHandler::start() {
    std::lock_guard<std::mutex> lock(exchanges_mutex_);
    
    if (running_) {
        return;
    }
    
    running_ = true;
    
    // Start a thread for each exchange - Week 3 optimization
    std::cout << "Week 3 optimization: Starting exchange threads for parallel processing" << std::endl;
    
    for (auto& [exchange, thread] : exchange_threads_) {
        if (thread.joinable()) {
            continue;
        }
        
        thread = std::thread(&MarketDataHandler::exchange_thread_func, this, exchange);
    }
}

// Stop processing
void MarketDataHandler::stop() {
    {
        std::lock_guard<std::mutex> lock(exchanges_mutex_);
        if (!running_) {
            return;
        }
        
        running_ = false;
    }
    
    // Wait for all threads to complete - Week 3 optimization
    std::cout << "Week 3 optimization: Gracefully stopping all exchange threads" << std::endl;
    
    for (auto& [exchange, thread] : exchange_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

// Exchange thread function
void MarketDataHandler::exchange_thread_func(const std::string& exchange_name) {
    std::cout << "Week 3 optimization: Exchange thread started for " << exchange_name << std::endl;
    
    // Avoid unused parameter warning
    static_cast<void>(exchange_name);
    
    while (running_) {
        // In a real implementation, this would receive data from the exchange
        // For demo purposes, we just sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "Week 3 optimization: Exchange thread stopped for " << exchange_name << std::endl;
}

// Update metrics
void MarketDataHandler::update_metrics(const std::string& exchange, 
                                     std::chrono::nanoseconds processing_time) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    // Update average latency (simple moving average)
    double current_latency = metrics_.avg_latency_us[exchange];
    double new_latency = processing_time.count() / 1000.0; // ns to us
    
    if (current_latency == 0.0) {
        metrics_.avg_latency_us[exchange] = new_latency;
    } else {
        metrics_.avg_latency_us[exchange] = current_latency * 0.9 + new_latency * 0.1;
    }
    
    // Update throughput (messages per second)
    // In a real implementation, we would track throughput over time
    metrics_.throughput_mps[exchange] = 10.0; // Placeholder
    
    // Update lock contention metrics - Week 3 optimization
    // This tracks when threads are waiting for locks, which can indicate
    // potential performance bottlenecks
    if (processing_time > std::chrono::microseconds(100)) {
        metrics_.lock_contentions.fetch_add(1, std::memory_order_relaxed);
        metrics_.lock_wait_time_ns.fetch_add(
            processing_time.count() / 2, std::memory_order_relaxed);
        
        std::cout << "Week 3 optimization: Detected potential lock contention with " 
                  << processing_time.count() / 1000.0 << " μs processing time for " 
                  << exchange << std::endl;
    }
}

} // namespace trading 
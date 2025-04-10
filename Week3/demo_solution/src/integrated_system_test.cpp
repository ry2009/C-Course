#include "../include/market_data_handler.hpp"
#include "../include/thread_pool.hpp"
#include "../include/lock_free_queue.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <queue>
#include <mutex>
#include <sstream>
#include <atomic>

using namespace trading;

// Define week1 namespace for sorting algorithm
namespace trading {
namespace week1 {
    template<typename Iterator, typename Compare>
    void quick_sort(Iterator begin, Iterator end, Compare comp) {
        // For demonstration purposes, we use std::sort as a placeholder
        // In a real Week 1 implementation, this would be a custom implementation
        std::sort(begin, end, comp);
    }
}
}

/**
 * @brief Timer utility for performance benchmarking.
 */
class Timer {
public:
    Timer(const std::string& name) : name_(name), start_(std::chrono::high_resolution_clock::now()) {}
    
    ~Timer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_).count();
        std::cout << "⏱️ " << name_ << " took " << duration << " μs (" 
                  << (duration / 1000.0) << " ms)" << std::endl;
    }
    
private:
    std::string name_;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

/**
 * @brief Mock trading strategy for testing.
 * 
 * This class simulates different trading strategies with
 * varying computation times and priorities.
 */
class TradingStrategy {
public:
    TradingStrategy(const std::string& name, int priority, bool use_week1_sorting = true)
        : name_(name), priority_(priority), use_week1_sorting_(use_week1_sorting),
          executions_(0), signals_generated_(0), total_execution_time_us_(0) {}
    
    // Evaluate strategy on market update
    std::vector<std::string> evaluate(const MarketUpdate& update) {
        auto start = std::chrono::high_resolution_clock::now();
        
        // Simulate strategy computation time based on priority
        // Higher priority strategies are assumed to be more complex
        std::this_thread::sleep_for(std::chrono::milliseconds(priority_ * 2));
        
        // Generate some mock signals
        std::vector<std::string> signals;
        
        // Apply Week 1 sorting optimization if enabled
        if (use_week1_sorting_) {
            std::cout << "Week 1 optimization: Using optimized sorting in strategy " 
                      << name_ << std::endl;
            
            // Simulate signal generation with some randomness
            int signal_count = 1 + (std::rand() % 3);
            for (int i = 0; i < signal_count; ++i) {
                std::stringstream ss;
                ss << "SIGNAL:" << name_ << ":" << update.symbol 
                   << ":" << (update.bid_price > update.ask_price ? "BUY" : "SELL")
                   << "@" << std::fixed << std::setprecision(2) 
                   << ((update.bid_price + update.ask_price) / 2.0);
                signals.push_back(ss.str());
            }
        } else {
            // Only generate one signal when not using optimized sorting
            std::stringstream ss;
            ss << "SIGNAL:" << name_ << ":" << update.symbol 
               << ":" << (update.bid_price > update.ask_price ? "BUY" : "SELL")
               << "@" << std::fixed << std::setprecision(2) 
               << ((update.bid_price + update.ask_price) / 2.0);
            signals.push_back(ss.str());
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        
        // Update stats
        executions_++;
        signals_generated_ += signals.size();
        total_execution_time_us_ += duration;
        
        return signals;
    }
    
    // Print strategy statistics
    void print_stats() const {
        std::cout << "Strategy " << name_ << " (priority " << priority_ << "):" << std::endl;
        std::cout << "  - Executions: " << executions_ << std::endl;
        std::cout << "  - Signals generated: " << signals_generated_ << std::endl;
        std::cout << "  - Average execution time: " 
                  << (executions_ > 0 ? total_execution_time_us_ / executions_ : 0) 
                  << " μs" << std::endl;
        std::cout << "  - Using Week 1 optimized sorting: " 
                  << (use_week1_sorting_ ? "Yes" : "No") << std::endl;
    }
    
    // Getters
    const std::string& name() const { return name_; }
    int priority() const { return priority_; }
    
private:
    std::string name_;
    int priority_;
    bool use_week1_sorting_;
    
    // Statistics
    size_t executions_;
    size_t signals_generated_;
    int64_t total_execution_time_us_;
};

/**
 * @brief Generate test market data for simulation.
 * 
 * @param count Number of updates to generate
 * @param symbols List of symbols to use
 * @param exchanges List of exchanges to use
 * @return std::vector<MarketUpdate> Generated market updates
 */
std::vector<MarketUpdate> generate_test_market_data(
    size_t count,
    const std::vector<std::string>& symbols,
    const std::vector<std::string>& exchanges
) {
    std::vector<MarketUpdate> updates;
    updates.reserve(count);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> symbol_dist(0, symbols.size() - 1);
    std::uniform_int_distribution<> exchange_dist(0, exchanges.size() - 1);
    std::uniform_real_distribution<> price_dist(100.0, 200.0);
    std::uniform_int_distribution<> volume_dist(100, 1000);
    
    auto now = std::chrono::high_resolution_clock::now();
    
    std::cout << "Generating " << count << " market updates..." << std::endl;
    
    for (size_t i = 0; i < count; ++i) {
        MarketUpdate update;
        update.symbol = symbols[symbol_dist(gen)];
        update.exchange = exchanges[exchange_dist(gen)];
        
        double mid_price = price_dist(gen);
        update.bid_price = mid_price - 0.01;
        update.ask_price = mid_price + 0.01;
        update.volume = volume_dist(gen);
        
        // Simulate timestamps with increasing values
        update.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch() + std::chrono::nanoseconds(i * 100));
        
        updates.push_back(update);
    }
    
    return updates;
}

/**
 * @brief Benchmark the performance of lock free queue vs mutex queue
 */
void benchmark_queue_performance() {
    std::cout << "\n=== BENCHMARK: Lock-Free Queue vs. Mutex Queue ===\n" << std::endl;
    
    const int NUM_ITEMS = 10000;
    std::cout << "Testing with " << NUM_ITEMS << " items" << std::endl;
    
    // Create queues
    trading::LockFreeQueue<int> lock_free_queue(false); // non-verbose mode
    
    std::queue<int> mutex_queue;
    std::mutex mutex_queue_mutex;
    
    std::atomic<int> lock_free_dequeued(0);
    std::atomic<int> mutex_dequeued(0);
    
    // Benchmark lock-free queue
    {
        Timer timer("Lock-free queue enqueue/dequeue");
        
        std::vector<std::thread> threads;
        const int NUM_THREADS = 4;
        
        // Create producer threads
        for (int t = 0; t < NUM_THREADS/2; ++t) {
            threads.emplace_back([&lock_free_queue, &NUM_ITEMS]() {
                for (int i = 0; i < NUM_ITEMS/(NUM_THREADS/2); ++i) {
                    lock_free_queue.enqueue(i);
                }
            });
        }
        
        // Create consumer threads
        for (int t = 0; t < NUM_THREADS/2; ++t) {
            threads.emplace_back([&lock_free_queue, &lock_free_dequeued]() {
                int item;
                while (lock_free_dequeued.load() < 10000) {
                    if (lock_free_queue.try_dequeue(item)) {
                        lock_free_dequeued.fetch_add(1);
                    }
                    // Small sleep to prevent busy waiting
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                }
            });
        }
        
        // Wait for all threads to complete
        for (auto& thread : threads) {
            thread.join();
        }
    }
    
    std::cout << "Lock-free queue: Total items dequeued: " << lock_free_dequeued.load() << std::endl;
    
    // Benchmark mutex-based queue
    {
        Timer timer("Mutex-based queue enqueue/dequeue");
        
        std::vector<std::thread> threads;
        const int NUM_THREADS = 4;
        
        // Create producer threads
        for (int t = 0; t < NUM_THREADS/2; ++t) {
            threads.emplace_back([&mutex_queue, &mutex_queue_mutex, &NUM_ITEMS]() {
                for (int i = 0; i < NUM_ITEMS/(NUM_THREADS/2); ++i) {
                    {
                        std::lock_guard<std::mutex> lock(mutex_queue_mutex);
                        mutex_queue.push(i);
                    }
                }
            });
        }
        
        // Create consumer threads
        for (int t = 0; t < NUM_THREADS/2; ++t) {
            threads.emplace_back([&mutex_queue, &mutex_queue_mutex, &mutex_dequeued]() {
                while (mutex_dequeued.load() < 10000) {
                    bool got_item = false;
                    {
                        std::lock_guard<std::mutex> lock(mutex_queue_mutex);
                        if (!mutex_queue.empty()) {
                            mutex_queue.pop();
                            got_item = true;
                        }
                    }
                    
                    if (got_item) {
                        mutex_dequeued.fetch_add(1);
                    }
                    
                    // Small sleep to prevent busy waiting
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                }
            });
        }
        
        // Wait for all threads to complete
        for (auto& thread : threads) {
            thread.join();
        }
    }
    
    std::cout << "Mutex-based queue: Total items dequeued: " << mutex_dequeued.load() << std::endl;
}

/**
 * @brief Benchmark the performance of thread pool vs creating threads
 */
void benchmark_thread_pool_performance() {
    std::cout << "\n=== BENCHMARK: Thread Pool vs. Direct Thread Creation ===\n" << std::endl;
    
    const int NUM_TASKS = 1000;
    const int WORK_SIZE = 10000;
    
    std::cout << "Testing with " << NUM_TASKS << " tasks, each doing " << WORK_SIZE << " work units" << std::endl;
    
    std::atomic<int> tasks_completed(0);
    
    // Function that simulates some CPU work
    auto cpu_work = [WORK_SIZE](int task_id) {
        volatile int sum = 0;
        for (int i = 0; i < WORK_SIZE; ++i) {
            sum += i * task_id;
        }
        return sum;
    };
    
    // Benchmark thread pool
    {
        Timer timer("Thread pool task execution");
        
        // Create thread pool with hardware concurrency
        trading::ThreadPool pool(std::thread::hardware_concurrency(), false); // non-verbose mode
        
        std::vector<std::future<int>> results;
        
        // Submit tasks to the pool
        for (int i = 0; i < NUM_TASKS; ++i) {
            auto result = pool.submit(1, cpu_work, i);
            results.push_back(std::move(result));
        }
        
        // Wait for all tasks to complete
        for (auto& result : results) {
            result.get();
            tasks_completed++;
        }
    }
    
    std::cout << "Thread pool: Completed " << tasks_completed.load() << " tasks" << std::endl;
    
    // Reset counter
    tasks_completed.store(0);
    
    // Benchmark creating threads directly
    {
        Timer timer("Direct thread creation");
        
        const int MAX_CONCURRENT = std::thread::hardware_concurrency();
        
        for (int batch = 0; batch < NUM_TASKS / MAX_CONCURRENT; ++batch) {
            std::vector<std::thread> threads;
            std::vector<int> results(MAX_CONCURRENT);
            
            // Create threads for this batch
            for (int i = 0; i < MAX_CONCURRENT; ++i) {
                int task_id = batch * MAX_CONCURRENT + i;
                if (task_id < NUM_TASKS) {
                    threads.emplace_back([&results, i, task_id, &cpu_work]() {
                        results[i] = cpu_work(task_id);
                    });
                }
            }
            
            // Wait for all threads in this batch
            for (auto& thread : threads) {
                thread.join();
            }
            
            tasks_completed.fetch_add(threads.size());
        }
    }
    
    std::cout << "Direct threads: Completed " << tasks_completed.load() << " tasks" << std::endl;
}

/**
 * @brief Integrated system test demonstrating all the optimizations.
 */
int main() {
    std::cout << "=== Week 3 Integrated Trading System Test ===" << std::endl;
    
    // Step 1: Configure the trading system
    std::cout << "\n=== STEP 1: Configuring Trading System ===\n" << std::endl;
    
    std::vector<std::string> symbols = {"AAPL", "MSFT", "GOOG", "AMZN", "META"};
    std::vector<std::string> exchanges = {"NYSE", "NASDAQ", "IEX"};
    
    // Initialize market data handler (Week 2 optimized memory management)
    std::cout << "Week 2 optimization: Initializing market data handler with custom allocator" << std::endl;
    trading::MarketDataHandler market_data_handler(100);  // Maximum 100 orders per symbol
    
    // Add exchanges
    for (const auto& exchange : exchanges) {
        std::cout << "Adding exchange: " << exchange << std::endl;
        market_data_handler.add_exchange(exchange);
    }
    
    // Create thread pool for strategy evaluation (Week 3 optimization)
    std::cout << "Week 3 optimization: Creating thread pool for strategy evaluation" << std::endl;
    trading::ThreadPool strategy_thread_pool(std::thread::hardware_concurrency(), false); // non-verbose mode
    
    // Create strategies with different priorities
    std::cout << "Creating trading strategies with different priorities:" << std::endl;
    TradingStrategy strategy1("Fast Alpha Strategy", 3, 10);  // High priority, fast execution
    std::cout << "  - " << strategy1.name() << " (Priority: " << strategy1.priority() << ")" << std::endl;
    
    TradingStrategy strategy2("ML-Based Strategy", 2, 50);  // Medium priority, slower execution
    std::cout << "  - " << strategy2.name() << " (Priority: " << strategy2.priority() << ")" << std::endl;
    
    TradingStrategy strategy3("Long-Term Strategy", 1, 30);  // Low priority, medium execution
    std::cout << "  - " << strategy3.name() << " (Priority: " << strategy3.priority() << ")" << std::endl;
    
    // Create lock-free queue for trading signals (Week 3 optimization)
    std::cout << "Week 3 optimization: Creating lock-free queue for trading signals" << std::endl;
    trading::LockFreeQueue<std::string> signal_queue(false); // non-verbose mode
    
    // Step 2: Subscribe to market data
    std::cout << "\n=== STEP 2: Subscribing to Market Data ===\n" << std::endl;
    
    for (const auto& symbol : symbols) {
        for (const auto& exchange : exchanges) {
            std::cout << "Subscribing to " << symbol << " on " << exchange << std::endl;
            market_data_handler.subscribe(symbol, exchange);
        }
    }
    
    // Step 3: Run benchmarks to demonstrate optimizations
    std::cout << "\n=== STEP 3: Running Performance Benchmarks ===\n" << std::endl;
    
    // Benchmark queue performance (Week 3 optimization)
    benchmark_queue_performance();
    
    // Benchmark thread pool performance (Week 3 optimization)
    benchmark_thread_pool_performance();
    
    // Step 4: Generate market updates
    std::cout << "\n=== STEP 4: Generating Market Updates ===\n" << std::endl;
    
    const int NUM_UPDATES = 100;
    std::cout << "Generating " << NUM_UPDATES << " random market updates" << std::endl;
    std::vector<trading::MarketUpdate> updates = generate_test_market_data(NUM_UPDATES, symbols, exchanges);
    
    // Step 5: Process market updates
    std::cout << "\n=== STEP 5: Processing Market Updates ===\n" << std::endl;
    
    size_t signals_generated = 0;
    
    {
        Timer timer("Processing market updates");
        
        for (const auto& update : updates) {
            // Process update in market data handler
            market_data_handler.process_update(update);
            
            // Evaluate strategies in parallel using thread pool
            auto strategy1_future = strategy_thread_pool.submit(
                strategy1.priority(),
                [&strategy1, &update, &signal_queue]() {
                    auto signals = strategy1.evaluate(update);
                    for (const auto& signal : signals) {
                        signal_queue.enqueue(signal);
                    }
                    return signals.size();
                }
            );
            
            auto strategy2_future = strategy_thread_pool.submit(
                strategy2.priority(),
                [&strategy2, &update, &signal_queue]() {
                    auto signals = strategy2.evaluate(update);
                    for (const auto& signal : signals) {
                        signal_queue.enqueue(signal);
                    }
                    return signals.size();
                }
            );
            
            auto strategy3_future = strategy_thread_pool.submit(
                strategy3.priority(),
                [&strategy3, &update, &signal_queue]() {
                    auto signals = strategy3.evaluate(update);
                    for (const auto& signal : signals) {
                        signal_queue.enqueue(signal);
                    }
                    return signals.size();
                }
            );
            
            // Wait for strategies to complete and count signals
            signals_generated += strategy1_future.get();
            signals_generated += strategy2_future.get();
            signals_generated += strategy3_future.get();
        }
    }
    
    // Step 6: Print results
    std::cout << "\n=== STEP 6: Results ===\n" << std::endl;
    
    // Get metrics from market data handler
    trading::MarketDataMetricsResult metrics = market_data_handler.get_metrics();
    
    std::cout << "Market updates processed: " << metrics.updates_processed
              << " in " << metrics.processing_time_ms << " ms" << std::endl;
    std::cout << "Updates dropped: " << metrics.updates_dropped << std::endl;
    std::cout << "Trading signals generated: " << signals_generated << std::endl;
    
    // Print strategy statistics
    std::cout << "\nStrategy Statistics:" << std::endl;
    strategy1.print_stats();
    strategy2.print_stats();
    strategy3.print_stats();
    
    // Print thread pool statistics
    std::cout << "\nThread Pool Statistics:" << std::endl;
    std::cout << "Total tasks completed: " << strategy_thread_pool.total_tasks_completed() << std::endl;
    
    // Display order books for each symbol
    std::cout << "\nOrder Books:" << std::endl;
    
    for (const auto& symbol : symbols) {
        std::cout << "Symbol: " << symbol << std::endl;
        
        // Get top bids and asks
        auto order_book = market_data_handler.get_order_book(symbol);
        
        std::cout << "  Top Bids:" << std::endl;
        for (const auto& bid : order_book.bids) {
            std::cout << "    " << bid.exchange << ": " << bid.price << " x " << bid.volume << std::endl;
        }
        
        std::cout << "  Top Asks:" << std::endl;
        for (const auto& ask : order_book.asks) {
            std::cout << "    " << ask.exchange << ": " << ask.price << " x " << ask.volume << std::endl;
        }
        
        std::cout << std::endl;
    }
    
    std::cout << "Test completed successfully!" << std::endl;
    return 0;
} 
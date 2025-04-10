/**
 * Threading Code Examples for Quantitative Finance
 * 
 * This file contains practical implementations of threading concepts
 * commonly used in trading systems.
 */

#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <vector>
#include <queue>
#include <functional>
#include <future>
#include <chrono>
#include <memory>
#include <string>
#include <algorithm>
#include <random>

using namespace std;
using namespace std::chrono;

/**
 * Example 1: Basic Thread Creation and Joining
 * Shows how to create threads and wait for their completion
 */
void basic_threading_example() {
    cout << "\n=== Basic Threading Example ===\n";
    
    // Function to be executed in a thread
    auto thread_func = [](int id) {
        cout << "Thread " << id << " starting\n";
        // Simulate work
        this_thread::sleep_for(milliseconds(100 * id));
        cout << "Thread " << id << " finishing\n";
    };
    
    // Create multiple threads
    vector<thread> threads;
    for (int i = 1; i <= 5; ++i) {
        threads.emplace_back(thread_func, i);
    }
    
    cout << "Main thread waiting for workers to finish...\n";
    
    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }
    
    cout << "All threads have completed\n";
}

/**
 * Example 2: Mutex for Thread Synchronization
 * Demonstrates protecting shared data with a mutex
 */
void mutex_synchronization_example() {
    cout << "\n=== Mutex Synchronization Example ===\n";
    
    int shared_counter = 0;
    mutex counter_mutex;
    
    auto increment_counter = [&](int id, int iterations) {
        for (int i = 0; i < iterations; ++i) {
            // Lock the mutex before accessing shared data
            lock_guard<mutex> lock(counter_mutex);
            ++shared_counter;
            cout << "Thread " << id << " incremented counter to " << shared_counter << "\n";
            this_thread::sleep_for(milliseconds(5)); // Small delay to simulate work
        }
    };
    
    // Create threads that will increment the counter
    vector<thread> threads;
    for (int i = 1; i <= 3; ++i) {
        threads.emplace_back(increment_counter, i, 3);
    }
    
    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }
    
    cout << "Final counter value: " << shared_counter << "\n";
}

/**
 * Example 3: Atomic Operations
 * Shows how atomic operations can be used without explicit locks
 */
void atomic_operations_example() {
    cout << "\n=== Atomic Operations Example ===\n";
    
    atomic<int> atomic_counter(0);
    int regular_counter = 0;
    
    const int iterations = 100000;
    const int num_threads = 10;
    
    auto increment_counters = [&]() {
        for (int i = 0; i < iterations; ++i) {
            // Atomic increment - thread safe without locks
            atomic_counter++;
            
            // Regular increment - not thread safe
            regular_counter++;
        }
    };
    
    // Create threads
    vector<thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(increment_counters);
    }
    
    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }
    
    cout << "Expected count: " << (iterations * num_threads) << "\n";
    cout << "Atomic counter: " << atomic_counter << " (correct due to atomic operations)\n";
    cout << "Regular counter: " << regular_counter << " (likely incorrect due to race conditions)\n";
}

/**
 * Example 4: Condition Variables for Thread Synchronization
 * Demonstrates using condition variables for thread signaling
 */
void condition_variable_example() {
    cout << "\n=== Condition Variable Example ===\n";
    
    // Shared state
    mutex mtx;
    condition_variable cv;
    bool market_data_ready = false;
    
    // Market data producer thread
    thread producer([&]() {
        cout << "Market data thread: Fetching data...\n";
        
        // Simulate market data retrieval
        this_thread::sleep_for(milliseconds(500));
        
        // Update shared state and notify consumer
        {
            lock_guard<mutex> lock(mtx);
            market_data_ready = true;
            cout << "Market data thread: Data is ready!\n";
        }
        
        cv.notify_one();  // Notify one waiting thread
    });
    
    // Strategy thread (consumer)
    thread consumer([&]() {
        cout << "Strategy thread: Waiting for market data...\n";
        
        // Wait for notification that data is ready
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [&]() { return market_data_ready; });
        
        cout << "Strategy thread: Processing market data and generating signals\n";
        // Process data...
        this_thread::sleep_for(milliseconds(200));
        cout << "Strategy thread: Done processing\n";
    });
    
    producer.join();
    consumer.join();
}

/**
 * Example 5: Thread Pool Implementation
 * Shows a simple thread pool for task distribution
 */
class ThreadPool {
private:
    vector<thread> workers;
    queue<function<void()>> tasks;
    
    mutex queue_mutex;
    condition_variable condition;
    bool stop;
    
public:
    ThreadPool(size_t num_threads) : stop(false) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    function<void()> task;
                    
                    {
                        unique_lock<mutex> lock(this->queue_mutex);
                        this->condition.wait(lock, [this] { 
                            return this->stop || !this->tasks.empty(); 
                        });
                        
                        if (this->stop && this->tasks.empty()) {
                            return;
                        }
                        
                        task = move(this->tasks.front());
                        this->tasks.pop();
                    }
                    
                    task();
                }
            });
        }
    }
    
    template<class F>
    void enqueue(F&& f) {
        {
            unique_lock<mutex> lock(queue_mutex);
            tasks.emplace(forward<F>(f));
        }
        condition.notify_one();
    }
    
    ~ThreadPool() {
        {
            unique_lock<mutex> lock(queue_mutex);
            stop = true;
        }
        
        condition.notify_all();
        
        for (auto& worker : workers) {
            worker.join();
        }
    }
};

void thread_pool_example() {
    cout << "\n=== Thread Pool Example ===\n";
    
    // Create a thread pool with 4 worker threads
    ThreadPool pool(4);
    
    // Simulate option pricing tasks
    auto price_option = [](int option_id, double strike) {
        auto thread_id = this_thread::get_id();
        cout << "Thread " << thread_id << " pricing option " << option_id 
             << " with strike " << strike << "\n";
        
        // Simulate computation time based on option complexity
        this_thread::sleep_for(milliseconds(100 + (option_id % 5) * 50));
        
        // Simulated result
        double price = 100.0 + (strike / 100.0) * (rand() % 10);
        
        cout << "Thread " << thread_id << " finished pricing option " << option_id 
             << ": price = " << price << "\n";
        
        return price;
    };
    
    // Store futures for the computations
    vector<future<double>> results;
    
    // Enqueue option pricing tasks
    for (int i = 0; i < 10; ++i) {
        // Use packaged_task to get a future
        packaged_task<double()> task([=] { 
            return price_option(i, 100.0 + i * 5.0); 
        });
        
        results.push_back(task.get_future());
        
        // Add the task to the pool
        pool.enqueue(move(task));
    }
    
    // Collect and display results
    cout << "Options pricing results:\n";
    for (size_t i = 0; i < results.size(); ++i) {
        cout << "Option " << i << " price: " << results[i].get() << "\n";
    }
}

/**
 * Example 6: Reader-Writer Lock
 * Demonstrates a reader-writer lock for allowing concurrent reads but exclusive writes
 */
class ReadWriteLock {
private:
    mutex mtx;
    condition_variable write_cv;
    condition_variable read_cv;
    int readers = 0;
    bool writer = false;
    int waiting_writers = 0;

public:
    void read_lock() {
        unique_lock<mutex> lock(mtx);
        // Wait if there's a writer or waiting writers
        while (writer || waiting_writers > 0) {
            read_cv.wait(lock);
        }
        readers++;
    }
    
    void read_unlock() {
        unique_lock<mutex> lock(mtx);
        readers--;
        if (readers == 0) {
            // If no more readers, notify a waiting writer
            write_cv.notify_one();
        }
    }
    
    void write_lock() {
        unique_lock<mutex> lock(mtx);
        waiting_writers++;
        // Wait until no readers and no active writer
        while (readers > 0 || writer) {
            write_cv.wait(lock);
        }
        waiting_writers--;
        writer = true;
    }
    
    void write_unlock() {
        unique_lock<mutex> lock(mtx);
        writer = false;
        
        // Notify waiting writers first (writer preference)
        if (waiting_writers > 0) {
            write_cv.notify_one();
        } else {
            // Otherwise notify all waiting readers
            read_cv.notify_all();
        }
    }
};

void reader_writer_lock_example() {
    cout << "\n=== Reader-Writer Lock Example ===\n";
    
    ReadWriteLock rwlock;
    vector<int> market_data = {100, 101, 102, 103, 104};
    
    // Reader function - reads market data
    auto reader_func = [&](int id) {
        for (int i = 0; i < 3; ++i) {
            rwlock.read_lock();
            
            cout << "Reader " << id << " reading market data: [ ";
            for (int price : market_data) {
                cout << price << " ";
                this_thread::sleep_for(milliseconds(10)); // Simulate read time
            }
            cout << "]\n";
            
            rwlock.read_unlock();
            
            // Sleep before next read
            this_thread::sleep_for(milliseconds(50));
        }
    };
    
    // Writer function - updates market data
    auto writer_func = [&](int id) {
        for (int i = 0; i < 2; ++i) {
            // Sleep to allow some reads to happen first
            this_thread::sleep_for(milliseconds(75));
            
            rwlock.write_lock();
            
            cout << "Writer " << id << " updating market data...\n";
            for (size_t j = 0; j < market_data.size(); ++j) {
                market_data[j] += (id * 10);
                this_thread::sleep_for(milliseconds(20)); // Simulate write time
            }
            cout << "Writer " << id << " finished update\n";
            
            rwlock.write_unlock();
        }
    };
    
    vector<thread> threads;
    
    // Start reader threads
    for (int i = 1; i <= 3; ++i) {
        threads.emplace_back(reader_func, i);
    }
    
    // Start writer threads
    for (int i = 1; i <= 2; ++i) {
        threads.emplace_back(writer_func, i);
    }
    
    // Wait for all threads to finish
    for (auto& t : threads) {
        t.join();
    }
}

/**
 * Example 7: Market Data Processing Pipeline
 * Demonstrates a multi-threaded pipeline for processing market data
 */
void market_data_pipeline_example() {
    cout << "\n=== Market Data Pipeline Example ===\n";
    
    // Shared queues between pipeline stages
    queue<string> raw_data_queue;
    queue<pair<string, double>> normalized_data_queue;
    queue<pair<string, double>> signal_queue;
    
    // Mutexes and condition variables for queues
    mutex raw_mutex, norm_mutex, signal_mutex;
    condition_variable raw_cv, norm_cv, signal_cv;
    
    // Control flags
    bool stop_pipeline = false;
    
    // Stage 1: Data Acquisition
    thread data_acquisition([&]() {
        cout << "Data Acquisition thread started\n";
        
        // Simulate market data feed
        vector<string> tickers = {"AAPL", "MSFT", "GOOGL", "AMZN", "FB"};
        
        for (int i = 0; i < 10 && !stop_pipeline; ++i) {
            for (const auto& ticker : tickers) {
                // Generate a raw data message
                string raw_message = ticker + ":PRICE:" + to_string(100 + (rand() % 100) / 10.0);
                
                {
                    lock_guard<mutex> lock(raw_mutex);
                    raw_data_queue.push(raw_message);
                    cout << "Acquired: " << raw_message << "\n";
                }
                
                raw_cv.notify_one();
                this_thread::sleep_for(milliseconds(20));
            }
        }
        
        cout << "Data Acquisition thread finishing\n";
    });
    
    // Stage 2: Data Normalization
    thread data_normalization([&]() {
        cout << "Data Normalization thread started\n";
        
        while (!stop_pipeline) {
            string raw_data;
            
            {
                unique_lock<mutex> lock(raw_mutex);
                raw_cv.wait_for(lock, milliseconds(100), [&]() {
                    return !raw_data_queue.empty() || stop_pipeline;
                });
                
                if (raw_data_queue.empty()) {
                    if (stop_pipeline) break;
                    continue;
                }
                
                raw_data = raw_data_queue.front();
                raw_data_queue.pop();
            }
            
            // Parse and normalize data
            size_t pos1 = raw_data.find(':');
            size_t pos2 = raw_data.find(':', pos1 + 1);
            
            string ticker = raw_data.substr(0, pos1);
            double price = stod(raw_data.substr(pos2 + 1));
            
            {
                lock_guard<mutex> lock(norm_mutex);
                normalized_data_queue.push({ticker, price});
                cout << "Normalized: " << ticker << " -> " << price << "\n";
            }
            
            norm_cv.notify_one();
            this_thread::sleep_for(milliseconds(15));
        }
        
        cout << "Data Normalization thread finishing\n";
    });
    
    // Stage 3: Signal Generation
    thread signal_generation([&]() {
        cout << "Signal Generation thread started\n";
        
        // Simple moving averages for each ticker
        unordered_map<string, vector<double>> price_history;
        
        while (!stop_pipeline) {
            pair<string, double> norm_data;
            
            {
                unique_lock<mutex> lock(norm_mutex);
                norm_cv.wait_for(lock, milliseconds(100), [&]() {
                    return !normalized_data_queue.empty() || stop_pipeline;
                });
                
                if (normalized_data_queue.empty()) {
                    if (stop_pipeline) break;
                    continue;
                }
                
                norm_data = normalized_data_queue.front();
                normalized_data_queue.pop();
            }
            
            string ticker = norm_data.first;
            double price = norm_data.second;
            
            // Update price history
            price_history[ticker].push_back(price);
            
            // Generate signal if we have enough history
            if (price_history[ticker].size() >= 3) {
                // Simple moving average
                double avg = 0;
                for (auto p : price_history[ticker]) {
                    avg += p;
                }
                avg /= price_history[ticker].size();
                
                // Simple signal: buy if current price < average, sell if > average
                string signal = (price < avg) ? "BUY" : "SELL";
                
                {
                    lock_guard<mutex> lock(signal_mutex);
                    signal_queue.push({ticker, price});
                    cout << "Signal generated for " << ticker << ": " << signal 
                         << " at " << price << " (avg: " << avg << ")\n";
                }
                
                signal_cv.notify_one();
                
                // Keep history limited
                if (price_history[ticker].size() > 5) {
                    price_history[ticker].erase(price_history[ticker].begin());
                }
            }
            
            this_thread::sleep_for(milliseconds(25));
        }
        
        cout << "Signal Generation thread finishing\n";
    });
    
    // Stage 4: Order Creation
    thread order_creation([&]() {
        cout << "Order Creation thread started\n";
        
        while (!stop_pipeline) {
            pair<string, double> signal_data;
            
            {
                unique_lock<mutex> lock(signal_mutex);
                signal_cv.wait_for(lock, milliseconds(100), [&]() {
                    return !signal_queue.empty() || stop_pipeline;
                });
                
                if (signal_queue.empty()) {
                    if (stop_pipeline) break;
                    continue;
                }
                
                signal_data = signal_queue.front();
                signal_queue.pop();
            }
            
            // Create order
            cout << "Order created for " << signal_data.first 
                 << " at $" << signal_data.second << "\n";
            
            this_thread::sleep_for(milliseconds(30));
        }
        
        cout << "Order Creation thread finishing\n";
    });
    
    // Let the pipeline run for a while
    this_thread::sleep_for(seconds(3));
    
    // Stop the pipeline
    stop_pipeline = true;
    raw_cv.notify_all();
    norm_cv.notify_all();
    signal_cv.notify_all();
    
    // Wait for all threads to finish
    data_acquisition.join();
    data_normalization.join();
    signal_generation.join();
    order_creation.join();
    
    cout << "Market data pipeline shut down\n";
}

/**
 * Example 8: Real-time Latency Monitoring
 * Demonstrates measuring and monitoring thread execution latency
 */
class LatencyMonitor {
private:
    mutex mtx;
    vector<microseconds> latencies;
    
public:
    void record_latency(microseconds latency) {
        lock_guard<mutex> lock(mtx);
        latencies.push_back(latency);
    }
    
    void print_stats() {
        lock_guard<mutex> lock(mtx);
        
        if (latencies.empty()) {
            cout << "No latency measurements recorded\n";
            return;
        }
        
        // Calculate statistics
        sort(latencies.begin(), latencies.end());
        
        auto min_latency = latencies.front();
        auto max_latency = latencies.back();
        
        microseconds sum(0);
        for (const auto& lat : latencies) {
            sum += lat;
        }
        
        auto avg_latency = microseconds(sum.count() / latencies.size());
        auto median_latency = latencies[latencies.size() / 2];
        
        // 99th percentile
        auto p99_latency = latencies[static_cast<size_t>(latencies.size() * 0.99)];
        
        // Print statistics
        cout << "\nLatency Statistics:\n";
        cout << "Min: " << min_latency.count() << " μs\n";
        cout << "Avg: " << avg_latency.count() << " μs\n";
        cout << "Median: " << median_latency.count() << " μs\n";
        cout << "Max: " << max_latency.count() << " μs\n";
        cout << "99th percentile: " << p99_latency.count() << " μs\n";
        cout << "Total measurements: " << latencies.size() << "\n";
    }
};

void latency_monitoring_example() {
    cout << "\n=== Latency Monitoring Example ===\n";
    
    LatencyMonitor monitor;
    
    // Simulate processing with varying latencies
    auto process_order = [&](int id) {
        auto thread_id = this_thread::get_id();
        
        for (int i = 0; i < 20; ++i) {
            auto start_time = high_resolution_clock::now();
            
            // Simulate processing with variable latency
            int base_latency = 100 + (id * 10);
            int jitter = rand() % 50;
            this_thread::sleep_for(microseconds(base_latency + jitter));
            
            auto end_time = high_resolution_clock::now();
            auto latency = duration_cast<microseconds>(end_time - start_time);
            
            // Record latency
            monitor.record_latency(latency);
            
            if (i % 5 == 0) {
                cout << "Thread " << thread_id << " processed order " << id << "-" << i 
                     << " with latency " << latency.count() << " μs\n";
            }
        }
    };
    
    // Create processing threads
    vector<thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(process_order, i);
    }
    
    // Wait for threads to complete
    for (auto& t : threads) {
        t.join();
    }
    
    // Print latency statistics
    monitor.print_stats();
}

/**
 * Example 9: Thread-safe Singleton with Double-Checked Locking
 * Demonstrates a thread-safe singleton implementation for system components
 */
class MarketDataService {
private:
    // Private constructor for singleton
    MarketDataService() {
        cout << "MarketDataService: Initializing...\n";
        this_thread::sleep_for(milliseconds(200));
        cout << "MarketDataService: Initialized\n";
    }
    
    // Delete copy constructor and assignment operator
    MarketDataService(const MarketDataService&) = delete;
    MarketDataService& operator=(const MarketDataService&) = delete;
    
    // Static instance and mutex
    static mutex instance_mutex;
    static unique_ptr<MarketDataService> instance;
    
public:
    // Thread-safe singleton access with double-checked locking
    static MarketDataService& get_instance() {
        MarketDataService* instance_ptr = instance.get();
        if (!instance_ptr) {
            lock_guard<mutex> lock(instance_mutex);
            instance_ptr = instance.get();
            if (!instance_ptr) {
                instance.reset(new MarketDataService());
                instance_ptr = instance.get();
            }
        }
        return *instance_ptr;
    }
    
    // Example method
    void subscribe(const string& symbol) {
        cout << "MarketDataService: Subscribed to " << symbol << "\n";
    }
};

// Initialize static members
mutex MarketDataService::instance_mutex;
unique_ptr<MarketDataService> MarketDataService::instance;

void thread_safe_singleton_example() {
    cout << "\n=== Thread-safe Singleton Example ===\n";
    
    auto access_service = [](int id) {
        cout << "Thread " << id << " accessing MarketDataService...\n";
        MarketDataService& service = MarketDataService::get_instance();
        
        // Simulate using the service
        service.subscribe("TICKER-" + to_string(id));
    };
    
    // Multiple threads try to access the singleton
    vector<thread> threads;
    for (int i = 1; i <= 5; ++i) {
        threads.emplace_back(access_service, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
}

/**
 * Example 10: Lock-free Queue Implementation
 * Demonstrates a simple lock-free queue for high-performance messaging
 */
template<typename T>
class LockFreeQueue {
private:
    struct Node {
        shared_ptr<T> data;
        atomic<Node*> next;
        
        Node() : next(nullptr) {}
    };
    
    atomic<Node*> head;
    atomic<Node*> tail;
    
    // Delete copy and assignment
    LockFreeQueue(const LockFreeQueue&) = delete;
    LockFreeQueue& operator=(const LockFreeQueue&) = delete;
    
public:
    LockFreeQueue() {
        Node* dummy = new Node();
        head.store(dummy);
        tail.store(dummy);
    }
    
    ~LockFreeQueue() {
        // Clean up nodes
        while (Node* node = head.load()) {
            head.store(node->next);
            delete node;
        }
    }
    
    void push(T item) {
        // Create new node with the data
        shared_ptr<T> new_data(make_shared<T>(move(item)));
        Node* new_node = new Node();
        new_node->data = new_data;
        
        // Add node to the end of the queue
        while (true) {
            Node* last = tail.load();
            Node* next = last->next.load();
            
            // Check if tail is still the last node
            if (last == tail.load()) {
                if (next == nullptr) {
                    // Try to link the new node at the end
                    if (last->next.compare_exchange_strong(next, new_node)) {
                        // Enqueue is done, try to update tail to the new node
                        tail.compare_exchange_strong(last, new_node);
                        return;
                    }
                } else {
                    // Tail is falling behind, try to update it
                    tail.compare_exchange_strong(last, next);
                }
            }
        }
    }
    
    bool try_pop(T& item) {
        while (true) {
            Node* first = head.load();
            Node* last = tail.load();
            Node* next = first->next.load();
            
            // Check if head is still the first node
            if (first == head.load()) {
                // If queue is empty
                if (first == last) {
                    if (next == nullptr) {
                        // Queue is empty
                        return false;
                    }
                    // Tail is falling behind, try to update it
                    tail.compare_exchange_strong(last, next);
                } else {
                    // Try to get the data
                    shared_ptr<T> data = next->data;
                    
                    // Try to remove the first node
                    if (head.compare_exchange_strong(first, next)) {
                        item = *data;
                        delete first;
                        return true;
                    }
                }
            }
        }
    }
    
    bool empty() {
        Node* first = head.load();
        Node* last = tail.load();
        return (first == last);
    }
};

void lock_free_queue_example() {
    cout << "\n=== Lock-free Queue Example ===\n";
    
    // Create a lock-free queue for market data messages
    LockFreeQueue<string> market_data_queue;
    
    // Flag to control threads
    atomic<bool> stop(false);
    
    // Producer thread that publishes market data
    thread producer([&]() {
        cout << "Producer thread started\n";
        
        vector<string> market_updates = {
            "AAPL:150.25", "MSFT:305.75", "GOOGL:2855.90", 
            "AMZN:3350.50", "FB:325.60", "TSLA:780.45"
        };
        
        int count = 0;
        while (!stop && count < 20) {
            // Publish a market update
            string update = market_updates[count % market_updates.size()];
            market_data_queue.push(update);
            
            cout << "Published: " << update << "\n";
            
            count++;
            this_thread::sleep_for(milliseconds(50));
        }
        
        cout << "Producer thread finishing\n";
    });
    
    // Consumer threads that process market data
    vector<thread> consumers;
    for (int i = 1; i <= 3; ++i) {
        consumers.emplace_back([&, i]() {
            cout << "Consumer " << i << " started\n";
            
            int processed_count = 0;
            while (!stop || !market_data_queue.empty()) {
                string update;
                
                if (market_data_queue.try_pop(update)) {
                    cout << "Consumer " << i << " processed: " << update << "\n";
                    processed_count++;
                    
                    // Simulate processing time
                    this_thread::sleep_for(milliseconds(25 + (i * 15)));
                } else {
                    // No data available, yield to other threads
                    this_thread::yield();
                    
                    // Small sleep to avoid busy waiting
                    this_thread::sleep_for(milliseconds(10));
                }
            }
            
            cout << "Consumer " << i << " finished, processed " << processed_count << " updates\n";
        });
    }
    
    // Let the example run for a while
    this_thread::sleep_for(seconds(2));
    
    // Signal threads to stop
    stop = true;
    
    // Wait for all threads to complete
    producer.join();
    for (auto& t : consumers) {
        t.join();
    }
    
    cout << "Lock-free queue example complete\n";
}

int main() {
    // Set random seed
    srand(static_cast<unsigned int>(time(nullptr)));
    
    cout << "=== C++ Threading Examples for Quantitative Finance ===\n";
    
    try {
        // Basic examples
        basic_threading_example();
        mutex_synchronization_example();
        atomic_operations_example();
        condition_variable_example();
        
        // Intermediate examples
        thread_pool_example();
        reader_writer_lock_example();
        
        // Advanced examples
        market_data_pipeline_example();
        latency_monitoring_example();
        thread_safe_singleton_example();
        lock_free_queue_example();
        
        cout << "\nAll examples completed successfully!\n";
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
} 
#pragma once

#include <atomic>
#include <memory>
#include <chrono>
#include <type_traits>
#include <optional>

namespace trading {

/**
 * @brief Performance metrics for the lock-free queue
 */
struct QueueMetrics {
    std::atomic<uint64_t> enqueue_count{0};
    std::atomic<uint64_t> dequeue_count{0};
    std::atomic<uint64_t> failed_dequeue_count{0};
    std::atomic<uint64_t> avg_enqueue_time_ns{0};
    std::atomic<uint64_t> avg_dequeue_time_ns{0};
    std::atomic<uint64_t> peak_size{0};
};

/**
 * @brief A lock-free queue implementation for inter-thread communication
 * 
 * This queue supports multiple producers and multiple consumers.
 * It is designed for high-performance scenarios where lock contention
 * would be a bottleneck.
 * 
 * @tparam T The type of elements stored in the queue
 */
template<typename T>
class LockFreeQueue {
public:
    /**
     * @brief Construct a new Lock Free Queue
     */
    LockFreeQueue() {
        // Initialize with a dummy node to simplify the algorithm
        Node* dummy = new Node();
        head_.store(dummy);
        tail_.store(dummy);
    }
    
    /**
     * @brief Destructor - cleans up all nodes
     */
    ~LockFreeQueue() {
        // Clean up all nodes
        T dummy;
        while (try_dequeue(dummy)) {
            // Continue dequeuing until empty
        }
        
        // Delete the remaining dummy node
        Node* dummy_node = head_.load();
        delete dummy_node;
    }
    
    // Delete copy constructor and assignment
    LockFreeQueue(const LockFreeQueue&) = delete;
    LockFreeQueue& operator=(const LockFreeQueue&) = delete;
    
    /**
     * @brief Enqueue an item
     * 
     * @param item The item to enqueue
     */
    void enqueue(T item) {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Create a new node with the item
        Node* new_node = new Node(std::move(item));
        
        // TODO: Implement the lock-free enqueue algorithm here
        // This is one of the key parts you need to implement
        
        // Update metrics
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
            end_time - start_time).count();
        
        // Update metrics atomically
        update_enqueue_metrics(duration);
    }
    
    /**
     * @brief Try to dequeue an item
     * 
     * @param item Reference to store the dequeued item
     * @return true if an item was dequeued, false if the queue was empty
     */
    bool try_dequeue(T& item) {
        auto start_time = std::chrono::high_resolution_clock::now();
        bool success = false;
        
        // TODO: Implement the lock-free dequeue algorithm here
        // This is one of the key parts you need to implement
        
        // Update metrics
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
            end_time - start_time).count();
        
        // Update metrics atomically
        update_dequeue_metrics(duration, success);
        
        return success;
    }
    
    /**
     * @brief Check if the queue is empty
     * 
     * Note: In a concurrent environment, this is just a snapshot
     * and may be immediately outdated.
     * 
     * @return true if the queue is empty
     */
    bool is_empty() const {
        return head_.load() == tail_.load();
    }
    
    /**
     * @brief Get the current queue metrics
     * 
     * @return Current queue metrics
     */
    QueueMetrics get_metrics() const {
        return metrics_;
    }
    
private:
    // Internal node structure
    struct Node {
        std::atomic<Node*> next{nullptr};
        std::optional<T> data;
        
        // Default constructor for dummy nodes
        Node() : data(std::nullopt) {}
        
        // Constructor with data
        explicit Node(T&& item) : data(std::move(item)) {}
    };
    
    // Queue pointers
    std::atomic<Node*> head_;
    std::atomic<Node*> tail_;
    
    // Performance metrics
    QueueMetrics metrics_;
    
    // Helper methods for updating metrics
    void update_enqueue_metrics(uint64_t duration_ns) {
        uint64_t count = metrics_.enqueue_count.fetch_add(1) + 1;
        uint64_t current_avg = metrics_.avg_enqueue_time_ns.load();
        
        // Update rolling average: new_avg = old_avg + (new_value - old_avg) / count
        uint64_t new_avg = current_avg + (duration_ns - current_avg) / count;
        metrics_.avg_enqueue_time_ns.store(new_avg);
        
        // Update peak size - approximate since we don't have a size() method
        uint64_t estimated_size = metrics_.enqueue_count.load() - metrics_.dequeue_count.load();
        uint64_t current_peak = metrics_.peak_size.load();
        if (estimated_size > current_peak) {
            metrics_.peak_size.store(estimated_size);
        }
    }
    
    void update_dequeue_metrics(uint64_t duration_ns, bool success) {
        if (success) {
            uint64_t count = metrics_.dequeue_count.fetch_add(1) + 1;
            uint64_t current_avg = metrics_.avg_dequeue_time_ns.load();
            
            // Update rolling average
            uint64_t new_avg = current_avg + (duration_ns - current_avg) / count;
            metrics_.avg_dequeue_time_ns.store(new_avg);
        } else {
            metrics_.failed_dequeue_count.fetch_add(1);
        }
    }
};

} // namespace trading 
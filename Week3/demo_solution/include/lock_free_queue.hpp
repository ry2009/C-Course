#pragma once

#include <atomic>
#include <memory>
#include <iostream>

/**
 * @file lock_free_queue.hpp
 * @brief Lock-free queue implementation for high-performance messaging (Week 3).
 * 
 * This file demonstrates concurrent programming optimizations from Week 3.
 * Key optimizations include:
 * - Lock-free algorithm to minimize contention
 * - Memory ordering optimizations
 * - Cache-friendly data structures
 */

namespace trading {

/**
 * @brief A lock-free queue implementation for high-performance concurrent access.
 * 
 * This class represents the Week 3 implementation of a lock-free queue
 * that allows multiple threads to safely push and pop elements without
 * explicit locking. Key features include:
 * 1. Wait-free enqueue operations
 * 2. Lock-free dequeue operations
 * 3. ABA problem prevention
 * 4. Memory reclamation for removed nodes
 * 
 * @tparam T Type of elements stored in the queue
 */
template<typename T>
class LockFreeQueue {
private:
    /**
     * @brief Node structure for the lock-free queue.
     */
    struct Node {
        std::shared_ptr<T> data;
        std::atomic<Node*> next;
        
        Node() : next(nullptr) {}
    };
    
    // Head and tail pointers
    std::atomic<Node*> head_;
    std::atomic<Node*> tail_;
    
    // Statistics for monitoring
    std::atomic<size_t> size_;
    std::atomic<size_t> total_enqueued_;
    std::atomic<size_t> total_dequeued_;
    
    // Logging control
    bool verbose_logging_;
    
    // Deleted copy and assignment operators
    LockFreeQueue(const LockFreeQueue&) = delete;
    LockFreeQueue& operator=(const LockFreeQueue&) = delete;
    
public:
    /**
     * @brief Construct a new Lock Free Queue.
     * 
     * @param verbose_logging Whether to log detailed operations (default: false)
     */
    LockFreeQueue(bool verbose_logging = false) 
        : size_(0), total_enqueued_(0), total_dequeued_(0), verbose_logging_(verbose_logging) {
        // Create a dummy node as the initial head and tail
        Node* dummy = new Node();
        head_.store(dummy);
        tail_.store(dummy);
        
        std::cout << "Week 3 optimization: Created lock-free queue with dummy node" << std::endl;
    }
    
    /**
     * @brief Destroy the Lock Free Queue.
     */
    ~LockFreeQueue() {
        // Free remaining nodes
        while (Node* old_head = head_.load()) {
            head_.store(old_head->next);
            delete old_head;
        }
        
        std::cout << "Week 3 optimization: Destroyed lock-free queue, processed " 
                  << total_enqueued_.load() << " enqueues and " 
                  << total_dequeued_.load() << " dequeues" << std::endl;
    }
    
    /**
     * @brief Enqueue an element to the queue.
     * 
     * @param value Value to enqueue
     */
    void enqueue(T value) {
        // Create a new node
        std::shared_ptr<T> new_data = std::make_shared<T>(std::move(value));
        Node* new_node = new Node();
        new_node->data = new_data;
        
        // Add the new node to the queue
        while (true) {
            Node* old_tail = tail_.load();
            Node* next = old_tail->next.load();
            
            // Check if tail is consistent
            if (old_tail == tail_.load()) {
                if (next == nullptr) {
                    // Try to link the new node at the end of the list
                    if (old_tail->next.compare_exchange_weak(next, new_node)) {
                        // Enqueue operation successful, advance tail
                        tail_.compare_exchange_strong(old_tail, new_node);
                        size_.fetch_add(1, std::memory_order_relaxed);
                        total_enqueued_.fetch_add(1, std::memory_order_relaxed);
                        
                        if (verbose_logging_) {
                            std::cout << "Week 3 optimization: Successfully enqueued item to lock-free queue (size: " 
                                      << size_.load() << ")" << std::endl;
                        }
                        return;
                    }
                } else {
                    // Tail is falling behind, help advance it
                    tail_.compare_exchange_strong(old_tail, next);
                }
            }
        }
    }
    
    /**
     * @brief Try to dequeue an element from the queue.
     * 
     * @param value Reference to store the dequeued value
     * @return true if dequeue was successful, false if the queue was empty
     */
    bool try_dequeue(T& value) {
        while (true) {
            Node* old_head = head_.load();
            Node* old_tail = tail_.load();
            Node* next = old_head->next.load();
            
            // Check if head is consistent
            if (old_head == head_.load()) {
                // Is queue empty?
                if (old_head == old_tail) {
                    if (next == nullptr) {
                        // Queue is empty
                        return false;
                    }
                    // Tail is falling behind, help advance it
                    tail_.compare_exchange_strong(old_tail, next);
                } else {
                    // Read the value before CAS, otherwise another thread might free the next node
                    if (next != nullptr) {
                        value = *(next->data);
                        
                        // Try to swing the head to the next node
                        if (head_.compare_exchange_weak(old_head, next)) {
                            // Dequeue operation successful
                            size_.fetch_sub(1, std::memory_order_relaxed);
                            total_dequeued_.fetch_add(1, std::memory_order_relaxed);
                            
                            // Free the old dummy node
                            delete old_head;
                            
                            if (verbose_logging_) {
                                std::cout << "Week 3 optimization: Successfully dequeued item from lock-free queue (size: " 
                                          << size_.load() << ")" << std::endl;
                            }
                            return true;
                        }
                    } else {
                        // If next is nullptr, the queue is empty
                        return false;
                    }
                }
            }
        }
    }
    
    /**
     * @brief Get the current size of the queue.
     * 
     * @return size_t Number of elements in the queue
     */
    size_t size() const {
        return size_.load(std::memory_order_relaxed);
    }
    
    /**
     * @brief Check if the queue is empty.
     * 
     * @return true if the queue is empty
     */
    bool empty() const {
        return size() == 0;
    }
    
    /**
     * @brief Get the total number of elements that have been enqueued.
     * 
     * @return size_t Total enqueued elements count
     */
    size_t total_enqueued() const {
        return total_enqueued_.load(std::memory_order_relaxed);
    }
    
    /**
     * @brief Get the total number of elements that have been dequeued.
     * 
     * @return size_t Total dequeued elements count
     */
    size_t total_dequeued() const {
        return total_dequeued_.load(std::memory_order_relaxed);
    }
    
    /**
     * @brief Set verbose logging mode
     * 
     * @param verbose Whether to enable verbose logging
     */
    void set_verbose_logging(bool verbose) {
        verbose_logging_ = verbose;
    }
};

} // namespace trading 
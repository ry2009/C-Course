#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>
#include <iostream>
#include <type_traits>  // for std::invoke_result

/**
 * @file thread_pool.hpp
 * @brief Priority-based thread pool implementation (Week 3).
 * 
 * This file demonstrates thread management optimizations from Week 3.
 * In a real implementation, this would include:
 * - Work stealing
 * - NUMA-awareness
 * - Cache-optimized task scheduling
 */

namespace trading {

/**
 * @brief A priority-based thread pool for parallel task execution.
 * 
 * This class represents the Week 3 implementation of a thread pool
 * that allows tasks to be submitted with different priorities.
 * Critical features include:
 * 1. Priority-based task scheduling
 * 2. Thread-safe task submission and execution
 * 3. Graceful shutdown
 * 4. Support for tasks with return values
 */
class ThreadPool {
public:
    /**
     * @brief Task with priority and function.
     */
    struct Task {
        int priority;  // Higher number = higher priority
        std::function<void()> func;
        
        // Comparison operator for priority queue
        bool operator<(const Task& other) const {
            return priority < other.priority;
        }
    };
    
    /**
     * @brief Construct a new Thread Pool.
     * 
     * @param num_threads Number of worker threads
     * @param verbose_logging Whether to log detailed operations (default: false)
     */
    explicit ThreadPool(size_t num_threads, bool verbose_logging = false)
        : stop_(false), active_tasks_(0), total_tasks_completed_(0), verbose_logging_(verbose_logging) {
        
        std::cout << "Week 3 optimization: Creating thread pool with " 
                  << num_threads << " threads" << std::endl;
        
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this, i] {
                worker_function(i);
            });
        }
    }
    
    /**
     * @brief Destroy the Thread Pool, stopping all threads.
     */
    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }
        
        condition_.notify_all();
        
        std::cout << "Week 3 optimization: Stopping thread pool, joining all threads" << std::endl;
        for (std::thread& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        
        std::cout << "Thread pool completed " << total_tasks_completed_.load() 
                  << " tasks in total" << std::endl;
    }
    
    /**
     * @brief Submit a task to the thread pool with a priority.
     * 
     * @tparam F Function type
     * @tparam Args Argument types
     * @param priority Task priority (higher number = higher priority)
     * @param f Function to execute
     * @param args Arguments to pass to the function
     * @return std::future<typename std::invoke_result<F, Args...>::type> Future result
     */
    template<class F, class... Args>
    auto submit(int priority, F&& f, Args&&... args) 
        -> std::future<typename std::invoke_result<F, Args...>::type> {
        
        using return_type = typename std::invoke_result<F, Args...>::type;
        
        // Create a shared pointer to the packaged task
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        // Get future result before enqueuing
        std::future<return_type> result = task->get_future();
        
        // Add task to the queue
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            
            // Don't allow enqueuing after stopping the pool
            if (stop_) {
                throw std::runtime_error("Cannot submit task to stopped ThreadPool");
            }
            
            // Wrap the packaged task in a void function
            tasks_.push(Task{
                priority,
                [task]() { (*task)(); }
            });
            
            active_tasks_.fetch_add(1, std::memory_order_relaxed);
        }
        
        // Notify one worker thread
        condition_.notify_one();
        
        if (verbose_logging_) {
            std::cout << "Week 3 optimization: Submitted task with priority " 
                    << priority << " to thread pool" << std::endl;
        }
        
        return result;
    }
    
    /**
     * @brief Get the number of worker threads.
     * 
     * @return size_t Number of worker threads
     */
    size_t size() const {
        return workers_.size();
    }
    
    /**
     * @brief Get the number of active tasks.
     * 
     * @return size_t Number of active tasks
     */
    size_t active_tasks() const {
        return active_tasks_.load(std::memory_order_relaxed);
    }
    
    /**
     * @brief Get the total number of completed tasks.
     * 
     * @return size_t Number of completed tasks
     */
    size_t total_tasks_completed() const {
        return total_tasks_completed_.load(std::memory_order_relaxed);
    }
    
    /**
     * @brief Set verbose logging mode
     * 
     * @param verbose Whether to enable verbose logging
     */
    void set_verbose_logging(bool verbose) {
        verbose_logging_ = verbose;
    }
    
private:
    /**
     * @brief Worker thread function.
     * 
     * @param id Worker thread ID
     */
    void worker_function(size_t id) {
        if (verbose_logging_) {
            std::cout << "Week 3 optimization: Thread pool worker " << id << " started" << std::endl;
        }
        
        while (true) {
            Task task;
            
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                
                // Wait for a task or stop signal
                condition_.wait(lock, [this] {
                    return stop_ || !tasks_.empty();
                });
                
                // If stopping and no tasks, exit
                if (stop_ && tasks_.empty()) {
                    if (verbose_logging_) {
                        std::cout << "Week 3 optimization: Thread pool worker " << id << " stopping" << std::endl;
                    }
                    break;
                }
                
                // Get task from queue
                task = tasks_.top();
                tasks_.pop();
            }
            
            // Execute the task
            if (verbose_logging_) {
                std::cout << "Week 3 optimization: Thread pool worker " << id 
                        << " executing task with priority " << task.priority << std::endl;
            }
            
            // Execute the task
            task.func();
            
            // Update counters
            active_tasks_.fetch_sub(1, std::memory_order_relaxed);
            total_tasks_completed_.fetch_add(1, std::memory_order_relaxed);
        }
    }
    
    // Worker threads
    std::vector<std::thread> workers_;
    
    // Task queue (priority queue)
    std::priority_queue<Task> tasks_;
    
    // Synchronization
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_;
    
    // Metrics
    std::atomic<size_t> active_tasks_;
    std::atomic<size_t> total_tasks_completed_;
    
    // Logging control
    bool verbose_logging_;
};

} // namespace trading 
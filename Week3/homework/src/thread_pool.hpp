#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <atomic>
#include <chrono>
#include <stdexcept>
#include <type_traits>

namespace trading {

/**
 * @brief Task priority levels for the thread pool
 */
enum class TaskPriority {
    LOW = 0,
    MEDIUM = 1,
    HIGH = 2
};

/**
 * @brief Statistics about the thread pool's operation
 */
struct ThreadPoolStats {
    std::atomic<uint64_t> tasks_completed{0};
    std::atomic<uint64_t> tasks_queued{0};
    std::atomic<uint64_t> tasks_rejected{0};
    std::atomic<uint64_t> avg_wait_time_us{0};
    std::atomic<uint64_t> avg_execution_time_us{0};
    
    // Per-priority stats
    std::array<std::atomic<uint64_t>, 3> tasks_by_priority{0, 0, 0};
    
    // Per-thread stats (indexed by thread ID)
    std::unordered_map<std::thread::id, uint64_t> tasks_per_thread;
};

/**
 * @brief Task wrapper with priority and timing information
 */
class TaskWrapper {
public:
    TaskWrapper(std::function<void()> task, TaskPriority priority, std::chrono::steady_clock::time_point enqueue_time)
        : task_(std::move(task)), priority_(priority), enqueue_time_(enqueue_time) {}
    
    void execute() {
        task_();
    }
    
    TaskPriority get_priority() const {
        return priority_;
    }
    
    std::chrono::steady_clock::time_point get_enqueue_time() const {
        return enqueue_time_;
    }
    
    // Comparison operator for priority queue
    bool operator<(const TaskWrapper& other) const {
        // Lower priority value means lower priority
        return static_cast<int>(priority_) < static_cast<int>(other.priority_);
    }
    
private:
    std::function<void()> task_;
    TaskPriority priority_;
    std::chrono::steady_clock::time_point enqueue_time_;
};

/**
 * @brief Thread pool for executing tasks in parallel
 * 
 * This thread pool supports:
 * - Dynamic sizing based on system load
 * - Task priorities (high, medium, low)
 * - Work stealing for load balancing
 * - Performance statistics
 */
class ThreadPool {
public:
    /**
     * @brief Construct a new Thread Pool
     * 
     * @param initial_threads Initial number of threads in the pool
     * @param max_threads Maximum number of threads the pool can grow to
     */
    ThreadPool(size_t initial_threads, size_t max_threads);
    
    /**
     * @brief Destructor - joins all threads
     */
    ~ThreadPool();
    
    // Delete copy constructor and assignment
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    
    /**
     * @brief Submit a task to the thread pool
     * 
     * @tparam F Function type
     * @tparam Args Argument types
     * @param priority Task priority
     * @param f Function to execute
     * @param args Arguments to pass to the function
     * @return std::future<return_type> Future for the task result
     */
    template<class F, class... Args>
    auto submit(TaskPriority priority, F&& f, Args&&... args) 
        -> std::future<std::invoke_result_t<F, Args...>> {
        
        using return_type = std::invoke_result_t<F, Args...>;
        
        // Create a packaged task
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        // Get the future before moving the task
        std::future<return_type> result = task->get_future();
        
        // Create a task wrapper
        auto enqueue_time = std::chrono::steady_clock::now();
        auto task_wrapper = [task]() { (*task)(); };
        
        // Add the task to the queue
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            
            // Check if the pool has been stopped
            if (stop_) {
                throw std::runtime_error("Cannot enqueue on stopped ThreadPool");
            }
            
            // Add the task to the queue
            task_queue_.emplace(std::move(task_wrapper), priority, enqueue_time);
            stats_.tasks_queued++;
            stats_.tasks_by_priority[static_cast<int>(priority)]++;
        }
        
        // Notify a waiting worker thread
        condition_.notify_one();
        
        // Check if we need to add more threads based on load
        adjust_thread_count();
        
        return result;
    }
    
    /**
     * @brief Get the current thread pool statistics
     * 
     * @return Current statistics
     */
    ThreadPoolStats get_stats() const;
    
    /**
     * @brief Resize the thread pool
     * 
     * @param new_size New number of threads
     */
    void resize(size_t new_size);
    
    /**
     * @brief Get the current number of threads
     * 
     * @return Number of threads
     */
    size_t size() const;
    
    /**
     * @brief Get the number of queued tasks
     * 
     * @return Number of tasks in the queue
     */
    size_t queue_size() const;
    
private:
    // TODO: Implement private member variables and methods
    
    // Worker thread function
    void worker_thread();
    
    // Adjust thread count based on load
    void adjust_thread_count();
    
    // Work stealing logic
    bool try_steal_task();
    
    // Track task execution time
    void track_task_execution(std::chrono::steady_clock::time_point start_time,
                             std::chrono::steady_clock::time_point end_time);
    
    // Member variables
    std::vector<std::thread> workers_;
    std::priority_queue<TaskWrapper> task_queue_;
    
    // Synchronization
    mutable std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_{false};
    
    // Thread pool configuration
    size_t max_threads_;
    
    // Statistics
    ThreadPoolStats stats_;
};

} // namespace trading 
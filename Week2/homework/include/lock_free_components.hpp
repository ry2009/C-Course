#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <optional>
#include <vector>

namespace trading {

// Order structures matching production system
struct Order {
    uint64_t order_id;
    std::string symbol;
    double price;
    uint64_t quantity;
    bool is_buy;
    std::chrono::nanoseconds timestamp;
    uint32_t client_id;
    uint16_t venue_id;
};

// Performance monitoring as used in production
struct QueueStats {
    std::atomic<uint64_t> enqueued_count{0};
    std::atomic<uint64_t> dequeued_count{0};
    std::atomic<uint64_t> current_size{0};
    std::atomic<uint64_t> peak_size{0};
    std::atomic<uint64_t> failed_enqueues{0};
    std::atomic<uint64_t> failed_dequeues{0};
    
    // Latency tracking (nanoseconds)
    std::atomic<uint64_t> min_enqueue_time{UINT64_MAX};
    std::atomic<uint64_t> max_enqueue_time{0};
    std::atomic<uint64_t> total_enqueue_time{0};
};

// Lock-free order queue from production
template<typename T>
class LockFreeQueue {
public:
    // Constructor with queue configuration
    explicit LockFreeQueue(size_t capacity);
    
    // Queue operations matching production
    bool enqueue(const T& item);
    std::optional<T> dequeue();
    bool try_enqueue(const T& item, std::chrono::nanoseconds timeout);
    std::optional<T> try_dequeue(std::chrono::nanoseconds timeout);
    
    // Bulk operations for efficiency
    size_t bulk_enqueue(const std::vector<T>& items);
    std::vector<T> bulk_dequeue(size_t max_items);
    
    // Queue management
    void clear();
    size_t size() const;
    bool empty() const;
    size_t capacity() const;
    
    // Performance monitoring
    const QueueStats& get_stats() const;
    void reset_stats();
    
private:
    // Implementation details to be filled by students
    // following the patterns from error_handling_trading.cpp
    
    // Node structure from production
    struct Node {
        static constexpr size_t CACHE_LINE_SIZE = 64;
        
        T data;
        std::atomic<Node*> next;
        uint8_t padding[CACHE_LINE_SIZE - sizeof(T) - sizeof(std::atomic<Node*>)];
    };
    static_assert(sizeof(Node) == Node::CACHE_LINE_SIZE,
                 "Node must be cache line aligned");
    
    // Queue state
    size_t capacity_;
    std::atomic<Node*> head_{nullptr};
    std::atomic<Node*> tail_{nullptr};
    QueueStats stats_;
    
    // Memory management
    std::vector<std::unique_ptr<Node>> nodes_;
    
    // Helper methods students need to implement
    Node* allocate_node();
    void deallocate_node(Node* node);
    bool try_link_nodes(Node* prev, Node* curr, Node* next);
};

// Lock-free allocator from production
template<typename T>
class LockFreeAllocator {
public:
    // Constructor with allocator configuration
    explicit LockFreeAllocator(size_t initial_size);
    
    // Allocation interface matching production
    T* allocate();
    void deallocate(T* ptr);
    
    // Memory management
    void reserve(size_t additional_size);
    size_t available() const;
    
    // Performance monitoring
    struct AllocStats {
        std::atomic<uint64_t> allocations{0};
        std::atomic<uint64_t> deallocations{0};
        std::atomic<uint64_t> current_size{0};
        std::atomic<uint64_t> peak_size{0};
        
        // Latency tracking (nanoseconds)
        std::atomic<uint64_t> min_allocation_time{UINT64_MAX};
        std::atomic<uint64_t> max_allocation_time{0};
        std::atomic<uint64_t> total_allocation_time{0};
    };
    
    const AllocStats& get_stats() const;
    void reset_stats();
    
private:
    // Implementation details to be filled by students
    // following the patterns from error_handling_trading.cpp
    
    // Memory block structure from production
    struct MemoryBlock {
        static constexpr size_t CACHE_LINE_SIZE = 64;
        
        void* data;
        size_t size;
        std::atomic<MemoryBlock*> next;
        uint8_t padding[CACHE_LINE_SIZE - 
                       sizeof(void*) - 
                       sizeof(size_t) - 
                       sizeof(std::atomic<MemoryBlock*>)];
    };
    static_assert(sizeof(MemoryBlock) == MemoryBlock::CACHE_LINE_SIZE,
                 "MemoryBlock must be cache line aligned");
    
    // Allocator state
    std::atomic<MemoryBlock*> free_blocks_{nullptr};
    AllocStats stats_;
    
    // Helper methods students need to implement
    MemoryBlock* allocate_block();
    void deallocate_block(MemoryBlock* block);
    bool try_link_blocks(MemoryBlock* prev, MemoryBlock* curr, MemoryBlock* next);
};

} // namespace trading 
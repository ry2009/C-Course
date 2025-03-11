#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <vector>
#include <queue>

namespace trading {

// Time series data structures matching production
struct TradeRecord {
    std::string symbol;
    double price;
    uint64_t volume;
    std::chrono::nanoseconds timestamp;
    uint32_t trade_id;
    uint16_t venue_id;
    uint8_t flags;
};

// Performance monitoring as used in production
struct PoolStats {
    std::atomic<uint64_t> allocations{0};
    std::atomic<uint64_t> deallocations{0};
    std::atomic<uint64_t> current_size{0};
    std::atomic<uint64_t> peak_size{0};
    std::atomic<uint64_t> cache_hits{0};
    std::atomic<uint64_t> cache_misses{0};
    
    // Latency tracking (nanoseconds)
    std::atomic<uint64_t> min_allocation_time{UINT64_MAX};
    std::atomic<uint64_t> max_allocation_time{0};
    std::atomic<uint64_t> total_allocation_time{0};
};

template<typename T>
class TimeSeriesPool {
public:
    // Constructor with pool configuration
    TimeSeriesPool(size_t initial_size, size_t block_size);
    
    // Allocation interface matching production
    T* allocate();
    void deallocate(T* ptr);
    
    // Bulk operations for efficiency
    std::vector<T*> bulk_allocate(size_t count);
    void bulk_deallocate(const std::vector<T*>& ptrs);
    
    // Pool management
    void reserve(size_t additional_size);
    size_t available() const;
    void clear();
    
    // Performance monitoring
    const PoolStats& get_stats() const;
    void reset_stats();
    
private:
    // Implementation details to be filled by students
    // following the patterns from memory_pool_trading.cpp
    
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
    
    // Lock-free recycling queue as used in production
    class RecycleQueue {
    public:
        void push(T* ptr);
        T* pop();
        bool empty() const;
        
    private:
        struct Node {
            T* data;
            std::atomic<Node*> next;
        };
        
        std::atomic<Node*> head_{nullptr};
        std::atomic<Node*> tail_{nullptr};
    };
    
    // Pool state
    size_t block_size_;
    std::atomic<MemoryBlock*> free_blocks_{nullptr};
    RecycleQueue recycle_queue_;
    PoolStats stats_;
    
    // Helper methods students need to implement
    MemoryBlock* allocate_block();
    void deallocate_block(MemoryBlock* block);
    T* get_from_recycle_queue();
    void add_to_recycle_queue(T* ptr);
};

} // namespace trading 
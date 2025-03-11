#pragma once

#include <atomic>
#include <array>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <vector>

namespace trading {

// Order types matching our production system
enum class OrderType : uint8_t {
    Market = 0,
    Limit = 1,
    Stop = 2,
    StopLimit = 3
};

// Memory block structure used in production
struct MemoryBlock {
    static constexpr size_t CACHE_LINE_SIZE = 64;
    void* data;
    size_t size;
    std::atomic<MemoryBlock*> next;
    uint8_t padding[CACHE_LINE_SIZE - sizeof(void*) - sizeof(size_t) - sizeof(std::atomic<MemoryBlock*>)];
};
static_assert(sizeof(MemoryBlock) == MemoryBlock::CACHE_LINE_SIZE, "MemoryBlock must be cache line aligned");

// Performance statistics tracking as used in production
struct AllocationStats {
    std::atomic<uint64_t> total_allocations{0};
    std::atomic<uint64_t> current_allocations{0};
    std::atomic<uint64_t> peak_allocations{0};
    std::atomic<uint64_t> total_bytes_allocated{0};
    std::atomic<uint64_t> allocation_failures{0};
    std::atomic<uint64_t> fragmentation_count{0};
    
    // Latency tracking (in nanoseconds)
    std::atomic<uint64_t> min_allocation_time{UINT64_MAX};
    std::atomic<uint64_t> max_allocation_time{0};
    std::atomic<uint64_t> total_allocation_time{0};
};

template<typename T>
class MultiLevelOrderBookAllocator {
public:
    // Constructor with initial pool size per order type
    explicit MultiLevelOrderBookAllocator(size_t initial_size_per_type);
    
    // Allocation methods matching production patterns
    T* allocate(OrderType type);
    void deallocate(T* ptr, OrderType type);
    
    // Bulk operations as used in production
    std::vector<T*> bulk_allocate(OrderType type, size_t count);
    void bulk_deallocate(const std::vector<T*>& ptrs, OrderType type);
    
    // Performance monitoring interface
    const AllocationStats& get_stats(OrderType type) const;
    void reset_stats(OrderType type);
    
    // Memory management
    void reserve(OrderType type, size_t additional_size);
    size_t available(OrderType type) const;
    
    // Thread safety operations
    void memory_fence() const noexcept;
    
private:
    // Implementation details to be filled by students
    // following the patterns from order_book_allocator.cpp
    
    // Free list per order type
    struct FreeList {
        std::atomic<MemoryBlock*> head{nullptr};
        AllocationStats stats;
        uint8_t padding[MemoryBlock::CACHE_LINE_SIZE - 
                       sizeof(std::atomic<MemoryBlock*>) - 
                       sizeof(AllocationStats)];
    };
    static_assert(sizeof(FreeList) == MemoryBlock::CACHE_LINE_SIZE, 
                 "FreeList must be cache line aligned");
    
    std::array<FreeList, 4> free_lists_;  // One per OrderType
    
    // Helper methods students need to implement
    MemoryBlock* allocate_block(size_t size);
    void deallocate_block(MemoryBlock* block);
    void add_to_free_list(OrderType type, MemoryBlock* block);
    MemoryBlock* get_from_free_list(OrderType type);
};

} // namespace trading 
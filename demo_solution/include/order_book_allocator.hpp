#pragma once

#include <iostream>
#include <cstddef>
#include <memory>

/**
 * @file order_book_allocator.hpp
 * @brief Custom memory allocator for OrderBook objects from Week 2.
 * 
 * This file demonstrates memory management optimizations from Week 2.
 * In a real implementation, this would include:
 * - Memory pooling to reduce fragmentation
 * - NUMA-aware allocation for multi-socket systems
 * - Lock-free memory allocation for concurrent access
 */

namespace trading {
namespace week2 {

/**
 * @brief Custom memory allocator for OrderBook objects.
 *
 * This class represents the Week 2 homework implementation of a specialized
 * memory allocator for financial data structures. In a real implementation,
 * this would provide significant performance benefits through:
 * 1. Pre-allocation of memory pools
 * 2. Cache-friendly memory layout
 * 3. Reduced system calls for memory allocation
 * 4. Minimized fragmentation
 */
class OrderBookAllocator {
public:
    /**
     * @brief Constructs a new OrderBookAllocator with specified capacity.
     * 
     * @param max_orders Maximum number of orders to support
     */
    explicit OrderBookAllocator(size_t max_orders) : max_orders_(max_orders) {
        std::cout << "Week 2 optimization: Initializing OrderBookAllocator with capacity for " 
                  << max_orders << " orders" << std::endl;
        // In a real implementation, we would pre-allocate memory pools here
    }
    
    /**
     * @brief Allocates memory of the specified size.
     * 
     * @param size Size of memory to allocate in bytes
     * @return void* Pointer to allocated memory
     */
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
    
    /**
     * @brief Deallocates previously allocated memory.
     * 
     * @param ptr Pointer to memory to deallocate
     */
    void deallocate(void* ptr) {
        deallocation_count_++;
        // For demonstration purposes, we use operator delete
        // In a real Week 2 implementation, this would return memory to the pool
        ::operator delete(ptr);
        
        // Log deallocation for demonstration purposes
        std::cout << "Week 2 optimization: Deallocated memory (total deallocations: " 
                  << deallocation_count_ << ")" << std::endl;
    }
    
    /**
     * @brief Gets the number of allocations made.
     * 
     * @return size_t Number of allocations
     */
    size_t get_allocation_count() const {
        return allocation_count_;
    }
    
    /**
     * @brief Gets the number of deallocations made.
     * 
     * @return size_t Number of deallocations
     */
    size_t get_deallocation_count() const {
        return deallocation_count_;
    }
    
    /**
     * @brief Gets the total number of bytes allocated.
     * 
     * @return size_t Total bytes allocated
     */
    size_t get_total_allocated() const {
        return total_allocated_;
    }
    
private:
    size_t max_orders_;          // Maximum number of orders to support
    size_t allocation_count_ = 0; // Number of allocations made
    size_t deallocation_count_ = 0; // Number of deallocations made
    size_t total_allocated_ = 0; // Total bytes allocated
};

} // namespace week2
} // namespace trading 
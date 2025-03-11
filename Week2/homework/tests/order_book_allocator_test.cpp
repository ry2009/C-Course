#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "order_book_allocator.hpp"

using namespace trading;

// Test fixture for order book allocator tests
class OrderBookAllocatorTest : public ::testing::Test {
protected:
    static constexpr size_t INITIAL_SIZE = 1024;
    
    struct OrderData {
        uint64_t order_id;
        double price;
        uint64_t quantity;
        bool is_buy;
    };
    
    MultiLevelOrderBookAllocator<OrderData> allocator{INITIAL_SIZE};
};

// Basic allocation tests
TEST_F(OrderBookAllocatorTest, BasicAllocation) {
    auto* order = allocator.allocate(OrderType::Market);
    ASSERT_NE(nullptr, order);
    order->order_id = 1;
    order->price = 100.0;
    order->quantity = 1000;
    order->is_buy = true;
    allocator.deallocate(order, OrderType::Market);
}

// Test allocation for different order types
TEST_F(OrderBookAllocatorTest, DifferentOrderTypes) {
    auto* market = allocator.allocate(OrderType::Market);
    auto* limit = allocator.allocate(OrderType::Limit);
    auto* stop = allocator.allocate(OrderType::Stop);
    auto* stop_limit = allocator.allocate(OrderType::StopLimit);
    
    ASSERT_NE(nullptr, market);
    ASSERT_NE(nullptr, limit);
    ASSERT_NE(nullptr, stop);
    ASSERT_NE(nullptr, stop_limit);
    
    allocator.deallocate(market, OrderType::Market);
    allocator.deallocate(limit, OrderType::Limit);
    allocator.deallocate(stop, OrderType::Stop);
    allocator.deallocate(stop_limit, OrderType::StopLimit);
}

// Test bulk allocation
TEST_F(OrderBookAllocatorTest, BulkAllocation) {
    static constexpr size_t BULK_SIZE = 100;
    
    auto orders = allocator.bulk_allocate(OrderType::Limit, BULK_SIZE);
    ASSERT_EQ(BULK_SIZE, orders.size());
    
    for (size_t i = 0; i < BULK_SIZE; ++i) {
        ASSERT_NE(nullptr, orders[i]);
        orders[i]->order_id = i;
        orders[i]->price = 100.0 + i;
        orders[i]->quantity = 1000 + i;
        orders[i]->is_buy = (i % 2 == 0);
    }
    
    allocator.bulk_deallocate(orders, OrderType::Limit);
}

// Test memory reuse
TEST_F(OrderBookAllocatorTest, MemoryReuse) {
    auto* first = allocator.allocate(OrderType::Market);
    auto first_address = reinterpret_cast<uintptr_t>(first);
    allocator.deallocate(first, OrderType::Market);
    
    auto* second = allocator.allocate(OrderType::Market);
    auto second_address = reinterpret_cast<uintptr_t>(second);
    
    EXPECT_EQ(first_address, second_address);
    allocator.deallocate(second, OrderType::Market);
}

// Test statistics tracking
TEST_F(OrderBookAllocatorTest, StatisticsTracking) {
    const auto& stats = allocator.get_stats(OrderType::Limit);
    auto initial_allocs = stats.total_allocations.load();
    
    auto* order = allocator.allocate(OrderType::Limit);
    ASSERT_NE(nullptr, order);
    
    EXPECT_EQ(initial_allocs + 1, stats.total_allocations.load());
    EXPECT_EQ(1u, stats.current_allocations.load());
    
    allocator.deallocate(order, OrderType::Limit);
    EXPECT_EQ(0u, stats.current_allocations.load());
}

// Test concurrent allocations
TEST_F(OrderBookAllocatorTest, ConcurrentAllocations) {
    static constexpr size_t NUM_THREADS = 4;
    static constexpr size_t ALLOCS_PER_THREAD = 1000;
    
    std::vector<std::thread> threads;
    std::atomic<size_t> successful_allocs{0};
    
    for (size_t i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([this, &successful_allocs]() {
            std::vector<OrderData*> orders;
            orders.reserve(ALLOCS_PER_THREAD);
            
            for (size_t j = 0; j < ALLOCS_PER_THREAD; ++j) {
                if (auto* order = allocator.allocate(OrderType::Market)) {
                    orders.push_back(order);
                    successful_allocs.fetch_add(1);
                }
            }
            
            for (auto* order : orders) {
                allocator.deallocate(order, OrderType::Market);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    const auto& stats = allocator.get_stats(OrderType::Market);
    EXPECT_EQ(successful_allocs.load(), stats.total_allocations.load());
    EXPECT_EQ(0u, stats.current_allocations.load());
}

// Test memory exhaustion
TEST_F(OrderBookAllocatorTest, MemoryExhaustion) {
    std::vector<OrderData*> orders;
    
    // Allocate until we run out of memory
    while (auto* order = allocator.allocate(OrderType::Limit)) {
        orders.push_back(order);
    }
    
    EXPECT_GT(orders.size(), 0);
    
    const auto& stats = allocator.get_stats(OrderType::Limit);
    EXPECT_GT(stats.allocation_failures.load(), 0);
    
    // Clean up
    for (auto* order : orders) {
        allocator.deallocate(order, OrderType::Limit);
    }
}

// Test memory reservation
TEST_F(OrderBookAllocatorTest, MemoryReservation) {
    static constexpr size_t ADDITIONAL_SIZE = 1024;
    
    size_t initial_available = allocator.available(OrderType::Stop);
    allocator.reserve(OrderType::Stop, ADDITIONAL_SIZE);
    
    EXPECT_EQ(initial_available + ADDITIONAL_SIZE, allocator.available(OrderType::Stop));
}

// Test memory fence operations
TEST_F(OrderBookAllocatorTest, MemoryFenceOperations) {
    auto* order = allocator.allocate(OrderType::Market);
    ASSERT_NE(nullptr, order);
    
    order->order_id = 1;
    allocator.memory_fence();
    EXPECT_EQ(1u, order->order_id);
    
    allocator.deallocate(order, OrderType::Market);
} 
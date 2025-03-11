#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <chrono>
#include "lock_free_components.hpp"

using namespace trading;
using namespace std::chrono_literals;

// Test fixture for lock-free queue tests
class LockFreeQueueTest : public ::testing::Test {
protected:
    static constexpr size_t QUEUE_CAPACITY = 1024;
    LockFreeQueue<Order> queue{QUEUE_CAPACITY};
};

// Basic queue operations
TEST_F(LockFreeQueueTest, BasicOperations) {
    Order order{
        .order_id = 1,
        .symbol = "AAPL",
        .price = 150.0,
        .quantity = 100,
        .is_buy = true,
        .timestamp = std::chrono::nanoseconds(1000000),
        .client_id = 1,
        .venue_id = 1
    };
    
    EXPECT_TRUE(queue.enqueue(order));
    EXPECT_EQ(1u, queue.size());
    
    auto result = queue.dequeue();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(order.order_id, result->order_id);
    EXPECT_EQ(order.symbol, result->symbol);
    EXPECT_EQ(order.price, result->price);
    EXPECT_EQ(order.quantity, result->quantity);
    EXPECT_EQ(order.is_buy, result->is_buy);
    EXPECT_EQ(order.timestamp, result->timestamp);
    EXPECT_EQ(order.client_id, result->client_id);
    EXPECT_EQ(order.venue_id, result->venue_id);
}

// Test queue capacity
TEST_F(LockFreeQueueTest, QueueCapacity) {
    EXPECT_EQ(QUEUE_CAPACITY, queue.capacity());
    
    // Fill queue to capacity
    for (size_t i = 0; i < QUEUE_CAPACITY; ++i) {
        Order order{.order_id = i};
        EXPECT_TRUE(queue.enqueue(order));
    }
    
    // Queue should be full
    Order order{.order_id = QUEUE_CAPACITY};
    EXPECT_FALSE(queue.enqueue(order));
}

// Test bulk operations
TEST_F(LockFreeQueueTest, BulkOperations) {
    static constexpr size_t BULK_SIZE = 100;
    
    std::vector<Order> orders;
    for (size_t i = 0; i < BULK_SIZE; ++i) {
        orders.push_back(Order{.order_id = i});
    }
    
    size_t enqueued = queue.bulk_enqueue(orders);
    EXPECT_EQ(BULK_SIZE, enqueued);
    
    auto dequeued = queue.bulk_dequeue(BULK_SIZE);
    EXPECT_EQ(BULK_SIZE, dequeued.size());
    
    for (size_t i = 0; i < BULK_SIZE; ++i) {
        EXPECT_EQ(i, dequeued[i].order_id);
    }
}

// Test concurrent operations
TEST_F(LockFreeQueueTest, ConcurrentOperations) {
    static constexpr size_t NUM_THREADS = 4;
    static constexpr size_t OPS_PER_THREAD = 10000;
    
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;
    std::atomic<size_t> enqueued{0};
    std::atomic<size_t> dequeued{0};
    
    // Start producer threads
    for (size_t i = 0; i < NUM_THREADS; ++i) {
        producers.emplace_back([this, &enqueued]() {
            for (size_t j = 0; j < OPS_PER_THREAD; ++j) {
                Order order{.order_id = j};
                if (queue.try_enqueue(order, 1ms)) {
                    enqueued.fetch_add(1);
                }
            }
        });
    }
    
    // Start consumer threads
    for (size_t i = 0; i < NUM_THREADS; ++i) {
        consumers.emplace_back([this, &dequeued]() {
            for (size_t j = 0; j < OPS_PER_THREAD; ++j) {
                if (auto result = queue.try_dequeue(1ms)) {
                    dequeued.fetch_add(1);
                }
            }
        });
    }
    
    // Wait for completion
    for (auto& thread : producers) {
        thread.join();
    }
    for (auto& thread : consumers) {
        thread.join();
    }
    
    EXPECT_EQ(enqueued.load(), dequeued.load());
    EXPECT_EQ(0u, queue.size());
}

// Test queue statistics
TEST_F(LockFreeQueueTest, QueueStatistics) {
    const auto& stats = queue.get_stats();
    auto initial_enqueued = stats.enqueued_count.load();
    
    Order order{.order_id = 1};
    queue.enqueue(order);
    
    EXPECT_EQ(initial_enqueued + 1, stats.enqueued_count.load());
    EXPECT_EQ(1u, stats.current_size.load());
    
    queue.dequeue();
    EXPECT_EQ(0u, stats.current_size.load());
}

// Test queue clear
TEST_F(LockFreeQueueTest, QueueClear) {
    for (size_t i = 0; i < 10; ++i) {
        Order order{.order_id = i};
        queue.enqueue(order);
    }
    
    EXPECT_EQ(10u, queue.size());
    queue.clear();
    EXPECT_EQ(0u, queue.size());
    EXPECT_TRUE(queue.empty());
}

// Test fixture for lock-free allocator tests
class LockFreeAllocatorTest : public ::testing::Test {
protected:
    static constexpr size_t INITIAL_SIZE = 1024;
    LockFreeAllocator<Order> allocator{INITIAL_SIZE};
};

// Basic allocation tests
TEST_F(LockFreeAllocatorTest, BasicAllocation) {
    auto* order = allocator.allocate();
    ASSERT_NE(nullptr, order);
    
    order->order_id = 1;
    order->symbol = "AAPL";
    order->price = 150.0;
    order->quantity = 100;
    order->is_buy = true;
    order->timestamp = std::chrono::nanoseconds(1000000);
    order->client_id = 1;
    order->venue_id = 1;
    
    allocator.deallocate(order);
}

// Test memory reuse
TEST_F(LockFreeAllocatorTest, MemoryReuse) {
    auto* first = allocator.allocate();
    auto first_address = reinterpret_cast<uintptr_t>(first);
    allocator.deallocate(first);
    
    auto* second = allocator.allocate();
    auto second_address = reinterpret_cast<uintptr_t>(second);
    
    EXPECT_EQ(first_address, second_address);
    allocator.deallocate(second);
}

// Test concurrent allocations
TEST_F(LockFreeAllocatorTest, ConcurrentAllocations) {
    static constexpr size_t NUM_THREADS = 4;
    static constexpr size_t ALLOCS_PER_THREAD = 1000;
    
    std::vector<std::thread> threads;
    std::atomic<size_t> successful_allocs{0};
    
    for (size_t i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([this, &successful_allocs]() {
            std::vector<Order*> orders;
            orders.reserve(ALLOCS_PER_THREAD);
            
            for (size_t j = 0; j < ALLOCS_PER_THREAD; ++j) {
                if (auto* order = allocator.allocate()) {
                    orders.push_back(order);
                    successful_allocs.fetch_add(1);
                }
            }
            
            for (auto* order : orders) {
                allocator.deallocate(order);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    const auto& stats = allocator.get_stats();
    EXPECT_EQ(successful_allocs.load(), stats.allocations.load());
    EXPECT_EQ(0u, stats.current_size.load());
}

// Test memory exhaustion
TEST_F(LockFreeAllocatorTest, MemoryExhaustion) {
    std::vector<Order*> orders;
    
    // Allocate until we run out of memory
    while (auto* order = allocator.allocate()) {
        orders.push_back(order);
    }
    
    EXPECT_GT(orders.size(), 0);
    
    // Clean up
    for (auto* order : orders) {
        allocator.deallocate(order);
    }
}

// Test memory reservation
TEST_F(LockFreeAllocatorTest, MemoryReservation) {
    static constexpr size_t ADDITIONAL_SIZE = 1024;
    
    size_t initial_available = allocator.available();
    allocator.reserve(ADDITIONAL_SIZE);
    
    EXPECT_EQ(initial_available + ADDITIONAL_SIZE, allocator.available());
}

// Test allocator statistics
TEST_F(LockFreeAllocatorTest, AllocatorStatistics) {
    const auto& stats = allocator.get_stats();
    auto initial_allocs = stats.allocations.load();
    
    auto* order = allocator.allocate();
    ASSERT_NE(nullptr, order);
    
    EXPECT_EQ(initial_allocs + 1, stats.allocations.load());
    EXPECT_EQ(1u, stats.current_size.load());
    
    allocator.deallocate(order);
    EXPECT_EQ(0u, stats.current_size.load());
}

// Test latency tracking
TEST_F(LockFreeAllocatorTest, LatencyTracking) {
    const auto& stats = allocator.get_stats();
    
    auto* order = allocator.allocate();
    ASSERT_NE(nullptr, order);
    
    EXPECT_GT(stats.total_allocation_time.load(), 0);
    EXPECT_GT(stats.max_allocation_time.load(), 0);
    EXPECT_LT(stats.min_allocation_time.load(), UINT64_MAX);
    
    allocator.deallocate(order);
} 
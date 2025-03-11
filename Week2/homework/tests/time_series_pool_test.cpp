#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "time_series_pool.hpp"

using namespace trading;

// Test fixture for time series pool tests
class TimeSeriesPoolTest : public ::testing::Test {
protected:
    static constexpr size_t INITIAL_SIZE = 1024;
    static constexpr size_t BLOCK_SIZE = 64;
    
    TimeSeriesPool<TradeRecord> pool{INITIAL_SIZE, BLOCK_SIZE};
};

// Basic allocation tests
TEST_F(TimeSeriesPoolTest, BasicAllocation) {
    auto* record = pool.allocate();
    ASSERT_NE(nullptr, record);
    
    record->symbol = "AAPL";
    record->price = 150.0;
    record->volume = 100;
    record->timestamp = std::chrono::nanoseconds(1000000);
    record->trade_id = 1;
    record->venue_id = 1;
    record->flags = 0;
    
    pool.deallocate(record);
}

// Test bulk allocation
TEST_F(TimeSeriesPoolTest, BulkAllocation) {
    static constexpr size_t BULK_SIZE = 100;
    
    auto records = pool.bulk_allocate(BULK_SIZE);
    ASSERT_EQ(BULK_SIZE, records.size());
    
    for (size_t i = 0; i < BULK_SIZE; ++i) {
        ASSERT_NE(nullptr, records[i]);
        records[i]->symbol = "AAPL";
        records[i]->price = 150.0 + i;
        records[i]->volume = 100 + i;
        records[i]->timestamp = std::chrono::nanoseconds(1000000 + i);
        records[i]->trade_id = i;
        records[i]->venue_id = 1;
        records[i]->flags = 0;
    }
    
    pool.bulk_deallocate(records);
}

// Test memory reuse
TEST_F(TimeSeriesPoolTest, MemoryReuse) {
    auto* first = pool.allocate();
    auto first_address = reinterpret_cast<uintptr_t>(first);
    pool.deallocate(first);
    
    auto* second = pool.allocate();
    auto second_address = reinterpret_cast<uintptr_t>(second);
    
    EXPECT_EQ(first_address, second_address);
    pool.deallocate(second);
}

// Test statistics tracking
TEST_F(TimeSeriesPoolTest, StatisticsTracking) {
    const auto& stats = pool.get_stats();
    auto initial_allocs = stats.allocations.load();
    
    auto* record = pool.allocate();
    ASSERT_NE(nullptr, record);
    
    EXPECT_EQ(initial_allocs + 1, stats.allocations.load());
    EXPECT_EQ(1u, stats.current_size.load());
    
    pool.deallocate(record);
    EXPECT_EQ(0u, stats.current_size.load());
}

// Test concurrent allocations
TEST_F(TimeSeriesPoolTest, ConcurrentAllocations) {
    static constexpr size_t NUM_THREADS = 4;
    static constexpr size_t ALLOCS_PER_THREAD = 1000;
    
    std::vector<std::thread> threads;
    std::atomic<size_t> successful_allocs{0};
    
    for (size_t i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([this, &successful_allocs]() {
            std::vector<TradeRecord*> records;
            records.reserve(ALLOCS_PER_THREAD);
            
            for (size_t j = 0; j < ALLOCS_PER_THREAD; ++j) {
                if (auto* record = pool.allocate()) {
                    records.push_back(record);
                    successful_allocs.fetch_add(1);
                }
            }
            
            for (auto* record : records) {
                pool.deallocate(record);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    const auto& stats = pool.get_stats();
    EXPECT_EQ(successful_allocs.load(), stats.allocations.load());
    EXPECT_EQ(0u, stats.current_size.load());
}

// Test memory exhaustion
TEST_F(TimeSeriesPoolTest, MemoryExhaustion) {
    std::vector<TradeRecord*> records;
    
    // Allocate until we run out of memory
    while (auto* record = pool.allocate()) {
        records.push_back(record);
    }
    
    EXPECT_GT(records.size(), 0);
    
    const auto& stats = pool.get_stats();
    EXPECT_GT(stats.cache_misses.load(), 0);
    
    // Clean up
    for (auto* record : records) {
        pool.deallocate(record);
    }
}

// Test memory reservation
TEST_F(TimeSeriesPoolTest, MemoryReservation) {
    static constexpr size_t ADDITIONAL_SIZE = 1024;
    
    size_t initial_available = pool.available();
    pool.reserve(ADDITIONAL_SIZE);
    
    EXPECT_EQ(initial_available + ADDITIONAL_SIZE, pool.available());
}

// Test cache performance
TEST_F(TimeSeriesPoolTest, CachePerformance) {
    const auto& stats = pool.get_stats();
    auto initial_hits = stats.cache_hits.load();
    auto initial_misses = stats.cache_misses.load();
    
    // First allocation should miss
    auto* record = pool.allocate();
    EXPECT_EQ(initial_misses + 1, stats.cache_misses.load());
    
    pool.deallocate(record);
    
    // Second allocation should hit
    record = pool.allocate();
    EXPECT_EQ(initial_hits + 1, stats.cache_hits.load());
    
    pool.deallocate(record);
}

// Test clear operation
TEST_F(TimeSeriesPoolTest, Clear) {
    std::vector<TradeRecord*> records;
    
    for (size_t i = 0; i < 10; ++i) {
        auto* record = pool.allocate();
        ASSERT_NE(nullptr, record);
        records.push_back(record);
    }
    
    pool.clear();
    
    const auto& stats = pool.get_stats();
    EXPECT_EQ(0u, stats.current_size.load());
    
    // Should be able to allocate again
    auto* record = pool.allocate();
    ASSERT_NE(nullptr, record);
    pool.deallocate(record);
}

// Test latency tracking
TEST_F(TimeSeriesPoolTest, LatencyTracking) {
    const auto& stats = pool.get_stats();
    
    auto* record = pool.allocate();
    ASSERT_NE(nullptr, record);
    
    EXPECT_GT(stats.total_allocation_time.load(), 0);
    EXPECT_GT(stats.max_allocation_time.load(), 0);
    EXPECT_LT(stats.min_allocation_time.load(), UINT64_MAX);
    
    pool.deallocate(record);
} 
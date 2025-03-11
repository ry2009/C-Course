#include <benchmark/benchmark.h>
#include <thread>
#include <vector>
#include "time_series_pool.hpp"

using namespace trading;

// Benchmark fixture for time series pool
class TimeSeriesPoolBench : public benchmark::Fixture {
protected:
    static constexpr size_t INITIAL_SIZE = 1024 * 1024;  // 1MB initial size
    static constexpr size_t BLOCK_SIZE = 64;  // 64-byte blocks
    
    void SetUp(const benchmark::State&) override {
        pool = std::make_unique<TimeSeriesPool<TradeRecord>>(INITIAL_SIZE, BLOCK_SIZE);
    }
    
    void TearDown(const benchmark::State&) override {
        pool.reset();
    }
    
    std::unique_ptr<TimeSeriesPool<TradeRecord>> pool;
};

// Benchmark single allocation/deallocation
BENCHMARK_F(TimeSeriesPoolBench, SingleAllocation)(benchmark::State& state) {
    for (auto _ : state) {
        auto* record = pool->allocate();
        benchmark::DoNotOptimize(record);
        pool->deallocate(record);
    }
    
    const auto& stats = pool->get_stats();
    state.counters["Allocations"] = benchmark::Counter(
        stats.allocations.load(),
        benchmark::Counter::kAvgIteration
    );
    state.counters["AvgLatency"] = benchmark::Counter(
        stats.total_allocation_time.load() / stats.allocations.load(),
        benchmark::Counter::kAvgIteration
    );
}

// Benchmark bulk allocation/deallocation
BENCHMARK_F(TimeSeriesPoolBench, BulkAllocation)(benchmark::State& state) {
    static constexpr size_t BULK_SIZE = 100;
    
    for (auto _ : state) {
        auto records = pool->bulk_allocate(BULK_SIZE);
        benchmark::DoNotOptimize(records);
        pool->bulk_deallocate(records);
    }
    
    const auto& stats = pool->get_stats();
    state.counters["Allocations"] = benchmark::Counter(
        stats.allocations.load(),
        benchmark::Counter::kAvgIteration
    );
    state.counters["AvgLatency"] = benchmark::Counter(
        stats.total_allocation_time.load() / stats.allocations.load(),
        benchmark::Counter::kAvgIteration
    );
}

// Benchmark concurrent allocations
BENCHMARK_F(TimeSeriesPoolBench, ConcurrentAllocations)(benchmark::State& state) {
    static constexpr size_t NUM_THREADS = 4;
    static constexpr size_t ALLOCS_PER_THREAD = 1000;
    
    for (auto _ : state) {
        std::vector<std::thread> threads;
        std::atomic<size_t> successful_allocs{0};
        
        for (size_t i = 0; i < NUM_THREADS; ++i) {
            threads.emplace_back([this, &successful_allocs]() {
                std::vector<TradeRecord*> records;
                records.reserve(ALLOCS_PER_THREAD);
                
                for (size_t j = 0; j < ALLOCS_PER_THREAD; ++j) {
                    if (auto* record = pool->allocate()) {
                        records.push_back(record);
                        successful_allocs.fetch_add(1);
                    }
                }
                
                for (auto* record : records) {
                    pool->deallocate(record);
                }
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        benchmark::DoNotOptimize(successful_allocs.load());
    }
    
    const auto& stats = pool->get_stats();
    state.counters["TotalAllocations"] = benchmark::Counter(
        stats.allocations.load(),
        benchmark::Counter::kAvgIteration
    );
}

// Benchmark memory reuse
BENCHMARK_F(TimeSeriesPoolBench, MemoryReuse)(benchmark::State& state) {
    static constexpr size_t NUM_RECORDS = 1000;
    std::vector<TradeRecord*> records;
    records.reserve(NUM_RECORDS);
    
    // Pre-allocate and deallocate to populate recycle queue
    for (size_t i = 0; i < NUM_RECORDS; ++i) {
        auto* record = pool->allocate();
        records.push_back(record);
    }
    for (auto* record : records) {
        pool->deallocate(record);
    }
    records.clear();
    
    // Benchmark memory reuse
    for (auto _ : state) {
        for (size_t i = 0; i < NUM_RECORDS; ++i) {
            auto* record = pool->allocate();
            benchmark::DoNotOptimize(record);
            records.push_back(record);
        }
        
        for (auto* record : records) {
            pool->deallocate(record);
        }
        records.clear();
    }
    
    const auto& stats = pool->get_stats();
    state.counters["CacheHits"] = benchmark::Counter(
        stats.cache_hits.load(),
        benchmark::Counter::kAvgIteration
    );
}

// Benchmark memory reservation
BENCHMARK_F(TimeSeriesPoolBench, MemoryReservation)(benchmark::State& state) {
    static constexpr size_t RESERVATION_SIZE = 1024;
    
    for (auto _ : state) {
        pool->reserve(RESERVATION_SIZE);
        benchmark::DoNotOptimize(pool->available());
    }
}

// Benchmark cache performance
BENCHMARK_F(TimeSeriesPoolBench, CachePerformance)(benchmark::State& state) {
    static constexpr size_t NUM_RECORDS = 1000;
    std::vector<TradeRecord*> records;
    records.reserve(NUM_RECORDS);
    
    for (auto _ : state) {
        // First allocation should miss cache
        auto* record = pool->allocate();
        benchmark::DoNotOptimize(record);
        pool->deallocate(record);
        
        // Second allocation should hit cache
        record = pool->allocate();
        benchmark::DoNotOptimize(record);
        pool->deallocate(record);
    }
    
    const auto& stats = pool->get_stats();
    state.counters["CacheHits"] = benchmark::Counter(
        stats.cache_hits.load(),
        benchmark::Counter::kAvgIteration
    );
    state.counters["CacheMisses"] = benchmark::Counter(
        stats.cache_misses.load(),
        benchmark::Counter::kAvgIteration
    );
}

// Benchmark clear operation
BENCHMARK_F(TimeSeriesPoolBench, Clear)(benchmark::State& state) {
    static constexpr size_t NUM_RECORDS = 1000;
    
    for (auto _ : state) {
        // Allocate some records
        for (size_t i = 0; i < NUM_RECORDS; ++i) {
            auto* record = pool->allocate();
            benchmark::DoNotOptimize(record);
        }
        
        // Clear the pool
        pool->clear();
        benchmark::DoNotOptimize(pool->available());
    }
}

// Run the benchmarks
BENCHMARK_MAIN(); 
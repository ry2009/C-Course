#include <benchmark/benchmark.h>
#include <thread>
#include <vector>
#include "order_book_allocator.hpp"

using namespace trading;

// Benchmark fixture for order book allocator
class OrderBookAllocatorBench : public benchmark::Fixture {
protected:
    static constexpr size_t INITIAL_SIZE = 1024 * 1024;  // 1MB initial size
    
    struct OrderData {
        uint64_t order_id;
        double price;
        uint64_t quantity;
        bool is_buy;
    };
    
    void SetUp(const benchmark::State&) override {
        allocator = std::make_unique<MultiLevelOrderBookAllocator<OrderData>>(INITIAL_SIZE);
    }
    
    void TearDown(const benchmark::State&) override {
        allocator.reset();
    }
    
    std::unique_ptr<MultiLevelOrderBookAllocator<OrderData>> allocator;
};

// Benchmark single allocation/deallocation
BENCHMARK_F(OrderBookAllocatorBench, SingleAllocation)(benchmark::State& state) {
    for (auto _ : state) {
        auto* order = allocator->allocate(OrderType::Market);
        benchmark::DoNotOptimize(order);
        allocator->deallocate(order, OrderType::Market);
    }
    
    const auto& stats = allocator->get_stats(OrderType::Market);
    state.counters["Allocations"] = benchmark::Counter(
        stats.total_allocations.load(),
        benchmark::Counter::kAvgIteration
    );
    state.counters["AvgLatency"] = benchmark::Counter(
        stats.total_allocation_time.load() / stats.total_allocations.load(),
        benchmark::Counter::kAvgIteration
    );
}

// Benchmark bulk allocation/deallocation
BENCHMARK_F(OrderBookAllocatorBench, BulkAllocation)(benchmark::State& state) {
    static constexpr size_t BULK_SIZE = 100;
    
    for (auto _ : state) {
        auto orders = allocator->bulk_allocate(OrderType::Limit, BULK_SIZE);
        benchmark::DoNotOptimize(orders);
        allocator->bulk_deallocate(orders, OrderType::Limit);
    }
    
    const auto& stats = allocator->get_stats(OrderType::Limit);
    state.counters["Allocations"] = benchmark::Counter(
        stats.total_allocations.load(),
        benchmark::Counter::kAvgIteration
    );
    state.counters["AvgLatency"] = benchmark::Counter(
        stats.total_allocation_time.load() / stats.total_allocations.load(),
        benchmark::Counter::kAvgIteration
    );
}

// Benchmark mixed order types
BENCHMARK_F(OrderBookAllocatorBench, MixedOrderTypes)(benchmark::State& state) {
    static constexpr size_t NUM_ORDERS = 100;
    std::vector<std::pair<OrderType, OrderData*>> orders;
    orders.reserve(NUM_ORDERS);
    
    for (auto _ : state) {
        for (size_t i = 0; i < NUM_ORDERS; ++i) {
            auto type = static_cast<OrderType>(i % 4);
            auto* order = allocator->allocate(type);
            benchmark::DoNotOptimize(order);
            orders.emplace_back(type, order);
        }
        
        for (const auto& [type, order] : orders) {
            allocator->deallocate(order, type);
        }
        orders.clear();
    }
}

// Benchmark concurrent allocations
BENCHMARK_F(OrderBookAllocatorBench, ConcurrentAllocations)(benchmark::State& state) {
    static constexpr size_t NUM_THREADS = 4;
    static constexpr size_t ALLOCS_PER_THREAD = 1000;
    
    for (auto _ : state) {
        std::vector<std::thread> threads;
        
        for (size_t i = 0; i < NUM_THREADS; ++i) {
            threads.emplace_back([this]() {
                std::vector<OrderData*> orders;
                orders.reserve(ALLOCS_PER_THREAD);
                
                for (size_t j = 0; j < ALLOCS_PER_THREAD; ++j) {
                    if (auto* order = allocator->allocate(OrderType::Market)) {
                        orders.push_back(order);
                    }
                }
                
                for (auto* order : orders) {
                    allocator->deallocate(order, OrderType::Market);
                }
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
    }
    
    const auto& stats = allocator->get_stats(OrderType::Market);
    state.counters["TotalAllocations"] = benchmark::Counter(
        stats.total_allocations.load(),
        benchmark::Counter::kAvgIteration
    );
}

// Benchmark memory reuse
BENCHMARK_F(OrderBookAllocatorBench, MemoryReuse)(benchmark::State& state) {
    static constexpr size_t NUM_ORDERS = 1000;
    std::vector<OrderData*> orders;
    orders.reserve(NUM_ORDERS);
    
    // Pre-allocate and deallocate to populate free list
    for (size_t i = 0; i < NUM_ORDERS; ++i) {
        auto* order = allocator->allocate(OrderType::Market);
        orders.push_back(order);
    }
    for (auto* order : orders) {
        allocator->deallocate(order, OrderType::Market);
    }
    orders.clear();
    
    // Benchmark memory reuse
    for (auto _ : state) {
        for (size_t i = 0; i < NUM_ORDERS; ++i) {
            auto* order = allocator->allocate(OrderType::Market);
            benchmark::DoNotOptimize(order);
            orders.push_back(order);
        }
        
        for (auto* order : orders) {
            allocator->deallocate(order, OrderType::Market);
        }
        orders.clear();
    }
    
    const auto& stats = allocator->get_stats(OrderType::Market);
    state.counters["CacheHits"] = benchmark::Counter(
        stats.total_allocations.load(),
        benchmark::Counter::kAvgIteration
    );
}

// Benchmark memory reservation
BENCHMARK_F(OrderBookAllocatorBench, MemoryReservation)(benchmark::State& state) {
    static constexpr size_t RESERVATION_SIZE = 1024;
    
    for (auto _ : state) {
        allocator->reserve(OrderType::Market, RESERVATION_SIZE);
        benchmark::DoNotOptimize(allocator->available(OrderType::Market));
    }
}

// Run the benchmarks
BENCHMARK_MAIN(); 
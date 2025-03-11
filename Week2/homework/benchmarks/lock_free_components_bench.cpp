#include <benchmark/benchmark.h>
#include <thread>
#include <vector>
#include <chrono>
#include "lock_free_components.hpp"

using namespace trading;
using namespace std::chrono_literals;

// Benchmark fixture for lock-free queue
class LockFreeQueueBench : public benchmark::Fixture {
protected:
    static constexpr size_t QUEUE_CAPACITY = 1024 * 1024;  // 1M elements
    LockFreeQueue<Order> queue{QUEUE_CAPACITY};
};

// Benchmark single enqueue/dequeue
BENCHMARK_F(LockFreeQueueBench, SingleOperation)(benchmark::State& state) {
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
    
    for (auto _ : state) {
        queue.enqueue(order);
        auto result = queue.dequeue();
        benchmark::DoNotOptimize(result);
    }
    
    const auto& stats = queue.get_stats();
    state.counters["Operations"] = benchmark::Counter(
        stats.enqueued_count.load(),
        benchmark::Counter::kAvgIteration
    );
}

// Benchmark bulk operations
BENCHMARK_F(LockFreeQueueBench, BulkOperations)(benchmark::State& state) {
    static constexpr size_t BULK_SIZE = 100;
    std::vector<Order> orders;
    
    for (size_t i = 0; i < BULK_SIZE; ++i) {
        orders.push_back(Order{.order_id = i});
    }
    
    for (auto _ : state) {
        auto enqueued = queue.bulk_enqueue(orders);
        auto dequeued = queue.bulk_dequeue(BULK_SIZE);
        benchmark::DoNotOptimize(dequeued);
    }
    
    const auto& stats = queue.get_stats();
    state.counters["Operations"] = benchmark::Counter(
        stats.enqueued_count.load(),
        benchmark::Counter::kAvgIteration
    );
}

// Benchmark producer/consumer pattern
BENCHMARK_F(LockFreeQueueBench, ProducerConsumer)(benchmark::State& state) {
    static constexpr size_t NUM_PRODUCERS = 4;
    static constexpr size_t NUM_CONSUMERS = 4;
    static constexpr size_t OPS_PER_PRODUCER = 10000;
    
    for (auto _ : state) {
        std::vector<std::thread> producers;
        std::vector<std::thread> consumers;
        std::atomic<size_t> produced{0};
        std::atomic<size_t> consumed{0};
        
        // Start producers
        for (size_t i = 0; i < NUM_PRODUCERS; ++i) {
            producers.emplace_back([this, &produced]() {
                for (size_t j = 0; j < OPS_PER_PRODUCER; ++j) {
                    Order order{.order_id = j};
                    if (queue.try_enqueue(order, 1ms)) {
                        produced.fetch_add(1);
                    }
                }
            });
        }
        
        // Start consumers
        for (size_t i = 0; i < NUM_CONSUMERS; ++i) {
            consumers.emplace_back([this, &consumed]() {
                while (consumed.load() < NUM_PRODUCERS * OPS_PER_PRODUCER) {
                    if (auto result = queue.try_dequeue(1ms)) {
                        consumed.fetch_add(1);
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
        
        benchmark::DoNotOptimize(produced.load());
        benchmark::DoNotOptimize(consumed.load());
    }
    
    const auto& stats = queue.get_stats();
    state.counters["Enqueued"] = benchmark::Counter(
        stats.enqueued_count.load(),
        benchmark::Counter::kAvgIteration
    );
    state.counters["Dequeued"] = benchmark::Counter(
        stats.dequeued_count.load(),
        benchmark::Counter::kAvgIteration
    );
}

// Benchmark queue contention
BENCHMARK_F(LockFreeQueueBench, QueueContention)(benchmark::State& state) {
    static constexpr size_t NUM_THREADS = 8;
    static constexpr size_t OPS_PER_THREAD = 10000;
    
    for (auto _ : state) {
        std::vector<std::thread> threads;
        
        for (size_t i = 0; i < NUM_THREADS; ++i) {
            threads.emplace_back([this]() {
                for (size_t j = 0; j < OPS_PER_THREAD; ++j) {
                    Order order{.order_id = j};
                    queue.enqueue(order);
                    auto result = queue.dequeue();
                    benchmark::DoNotOptimize(result);
                }
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
    }
    
    const auto& stats = queue.get_stats();
    state.counters["Operations"] = benchmark::Counter(
        stats.enqueued_count.load(),
        benchmark::Counter::kAvgIteration
    );
}

// Benchmark fixture for lock-free allocator
class LockFreeAllocatorBench : public benchmark::Fixture {
protected:
    static constexpr size_t INITIAL_SIZE = 1024 * 1024;  // 1MB initial size
    LockFreeAllocator<Order> allocator{INITIAL_SIZE};
};

// Benchmark single allocation/deallocation
BENCHMARK_F(LockFreeAllocatorBench, SingleAllocation)(benchmark::State& state) {
    for (auto _ : state) {
        auto* order = allocator.allocate();
        benchmark::DoNotOptimize(order);
        allocator.deallocate(order);
    }
    
    const auto& stats = allocator.get_stats();
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
BENCHMARK_F(LockFreeAllocatorBench, ConcurrentAllocations)(benchmark::State& state) {
    static constexpr size_t NUM_THREADS = 4;
    static constexpr size_t ALLOCS_PER_THREAD = 1000;
    
    for (auto _ : state) {
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
        
        benchmark::DoNotOptimize(successful_allocs.load());
    }
    
    const auto& stats = allocator.get_stats();
    state.counters["TotalAllocations"] = benchmark::Counter(
        stats.allocations.load(),
        benchmark::Counter::kAvgIteration
    );
}

// Benchmark memory reuse
BENCHMARK_F(LockFreeAllocatorBench, MemoryReuse)(benchmark::State& state) {
    static constexpr size_t NUM_ORDERS = 1000;
    std::vector<Order*> orders;
    orders.reserve(NUM_ORDERS);
    
    // Pre-allocate and deallocate to populate free list
    for (size_t i = 0; i < NUM_ORDERS; ++i) {
        auto* order = allocator.allocate();
        orders.push_back(order);
    }
    for (auto* order : orders) {
        allocator.deallocate(order);
    }
    orders.clear();
    
    // Benchmark memory reuse
    for (auto _ : state) {
        for (size_t i = 0; i < NUM_ORDERS; ++i) {
            auto* order = allocator.allocate();
            benchmark::DoNotOptimize(order);
            orders.push_back(order);
        }
        
        for (auto* order : orders) {
            allocator.deallocate(order);
        }
        orders.clear();
    }
}

// Benchmark memory reservation
BENCHMARK_F(LockFreeAllocatorBench, MemoryReservation)(benchmark::State& state) {
    static constexpr size_t RESERVATION_SIZE = 1024;
    
    for (auto _ : state) {
        allocator.reserve(RESERVATION_SIZE);
        benchmark::DoNotOptimize(allocator.available());
    }
}

// Run the benchmarks
BENCHMARK_MAIN(); 
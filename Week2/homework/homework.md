# Week 2 Homework: Memory Management in Trading System Infrastructure

## Overview

In this homework assignment, you will implement core memory management components used in our trading system infrastructure. You'll work with the same patterns and techniques demonstrated in the order book allocator, smart pointer trading system, and memory pool examples from our infrastructure.

## Part 1: Order Book Memory Management (40 points)

Extend the `OrderBookAllocator` from our trading system to support multi-level order books with different order types.

### Requirements:

1. (15 points) Implement the order book allocator based on our production system:
   - Use the same memory block structure as shown in `order_book_allocator.cpp`
   - Support the four order types: Market, Limit, Stop, Stop-Limit
   - Implement the free list recycling mechanism
   - Add support for bulk operations

2. (15 points) Add the performance monitoring features:
   - Track allocation patterns per order type
   - Monitor memory fragmentation
   - Implement the same statistics tracking as our production system
   - Add latency measurements for allocations

3. (10 points) Implement thread safety features used in our system:
   - Use the same atomic operations for counters
   - Implement the lock-free free list
   - Add the memory fence operations as shown in the examples

## Part 2: Market Data Memory Safety (30 points)

Implement the market data management system using our production patterns from `smart_pointer_trading.cpp`.

### Requirements:

1. (15 points) Create the market data subscription system:
   - Use the same RAII subscription pattern
   - Implement the weak_ptr pattern for market data feeds
   - Add the reference counting mechanism
   - Support multiple data sources

2. (15 points) Implement the error handling system:
   - Use the same exception hierarchy
   - Add the automatic cleanup on errors
   - Implement the reconnection logic
   - Add the same logging and monitoring

## Part 3: Time Series Data Pool (30 points)

Implement the memory pool system for time series data based on our `memory_pool_trading.cpp` example.

### Requirements:

1. (15 points) Create the time series pool:
   - Use the same block structure as our production system
   - Implement the fixed-size allocation strategy
   - Add support for trade record recycling
   - Include the performance monitoring

2. (15 points) Add the multi-threading support:
   - Implement the same lock-free queue
   - Add the thread-safe processing
   - Use atomic operations for counters
   - Include the same error handling patterns

## Bonus Challenge: Lock-Free Trading Components (20 points)

Implement the lock-free components from our error handling system in `error_handling_trading.cpp`:

1. (10 points) Create the lock-free order queue:
   - Use the same atomic operations
   - Implement the same error recovery
   - Add performance monitoring
   - Support multiple producers/consumers

2. (10 points) Add the advanced features:
   - Implement the same memory fence operations
   - Add the cache line padding
   - Include latency measurements
   - Support order cancellation

## Submission Requirements

1. Code Implementation:
   - Follow our coding standards
   - Use the same naming conventions
   - Include the same error handling patterns
   - Add comprehensive unit tests

2. Documentation:
   - Explain your implementation choices
   - Document performance characteristics
   - Include benchmark results
   - Provide usage examples

3. Performance Analysis:
   - Compare with standard allocators
   - Measure latency distributions
   - Analyze memory usage patterns
   - Profile cache performance

## Grading Criteria

- Correctness (40%): Matches our production patterns
- Performance (30%): Meets latency requirements
- Design (20%): Follows system architecture
- Documentation (10%): Clear and complete

## Notes

- Use C++17 features as shown in examples
- Follow our error handling patterns
- Use the same atomic operations
- Match production logging format

## Resources

- Trading system example code
- Infrastructure documentation
- Performance requirements doc
- System architecture guide

## Due Date

Submit your completed assignment by 11:59 PM next Friday.

## Academic Integrity

- All work must be your own
- You may reference the provided examples
- Do not share your implementation
- Document any external resources used 
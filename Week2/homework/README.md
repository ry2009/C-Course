# Week 2 Homework: Memory Management in Trading Systems

## Overview

This homework assignment focuses on implementing core memory management components used in our trading system infrastructure. You will work with the same patterns and techniques used in our production system, including custom allocators, smart pointers, memory pools, and lock-free data structures.

## Project Structure

```
Week2/homework/
├── include/                    # Header files
│   ├── order_book_allocator.hpp   # Multi-level order book allocator
│   ├── market_data_manager.hpp    # Market data subscription system
│   ├── time_series_pool.hpp       # Time series data memory pool
│   └── lock_free_components.hpp   # Lock-free trading components
├── src/                       # Source files
│   ├── order_book_allocator.cpp
│   ├── market_data_manager.cpp
│   ├── time_series_pool.cpp
│   └── lock_free_components.cpp
├── tests/                    # Unit tests
│   ├── order_book_allocator_test.cpp
│   ├── market_data_manager_test.cpp
│   ├── time_series_pool_test.cpp
│   └── lock_free_components_test.cpp
├── benchmarks/              # Performance benchmarks
│   ├── order_book_allocator_bench.cpp
│   ├── market_data_manager_bench.cpp
│   ├── time_series_pool_bench.cpp
│   └── lock_free_components_bench.cpp
├── docs/                    # Documentation
├── CMakeLists.txt          # Build configuration
└── README.md               # This file
```

## Requirements

- C++17 compatible compiler (same as production)
- CMake 3.10 or higher
- Google Test (for unit testing)
- Google Benchmark (for performance testing)
- Doxygen (for documentation)
- Threading support

## Building the Project

```bash
# Create build directory
mkdir build && cd build

# Generate build files
cmake ..

# Build the project
cmake --build .

# This will create three executables in the bin directory:
# - homework_solution
# - homework_tests
# - homework_benchmarks
```

## Running Tests

```bash
# Run unit tests
./bin/homework_tests

# Run benchmarks
./bin/homework_benchmarks
```

## Generating Documentation

```bash
# Generate documentation using Doxygen
cmake --build . --target doc
```

## Implementation Details

### MultiLevelOrderBookAllocator
- Custom allocator for order book data structures
- Supports different order types (Market, Limit, Stop, StopLimit)
- Uses free list recycling for memory reuse
- Implements thread-safe operations with atomic counters
- Includes performance monitoring and statistics tracking

### MarketDataManager
- RAII-based subscription management
- Uses weak_ptr pattern for market data feeds
- Implements automatic cleanup on errors
- Includes reconnection logic and monitoring

### TimeSeriesPool
- Fixed-size allocation strategy for time series data
- Lock-free recycling queue for trade records
- Cache-aligned memory blocks
- Performance monitoring with latency tracking

### LockFreeComponents
- Lock-free order queue implementation
- Memory fence operations for consistency
- Cache line padding for performance
- Latency measurement and monitoring

## Performance Considerations

### Memory Allocation
- Use of cache-aligned structures
- Bulk operations for efficiency
- Memory recycling to reduce allocations
- Fixed-size blocks for predictable performance

### Thread Safety
- Lock-free algorithms where possible
- Atomic operations for counters
- Memory fences for consistency
- Cache line padding to prevent false sharing

### Cache Efficiency
- Aligned memory blocks
- Contiguous memory allocation
- Minimized pointer chasing
- Efficient data structure layout

## Testing Strategy

### Unit Tests
- Test all allocation/deallocation paths
- Verify thread safety
- Check error handling
- Validate memory cleanup

### Performance Tests
- Measure allocation latencies
- Test concurrent operations
- Verify memory usage patterns
- Compare with standard allocators

## Known Limitations

1. Memory Pools
   - Fixed block sizes
   - No coalescing of free blocks
   - Memory fragmentation possible

2. Lock-Free Allocator
   - Limited to power-of-2 sizes
   - No compaction support
   - Memory overhead for padding

3. Market Data Management
   - Single-threaded subscription updates
   - Limited error recovery options
   - Fixed reconnection strategy

## Future Improvements

1. Performance
   - Add memory compaction
   - Implement block splitting
   - Optimize cache usage

2. Features
   - Add memory usage predictions
   - Implement adaptive block sizes
   - Add more error recovery options

3. Testing
   - Add stress tests
   - Improve benchmark coverage
   - Add memory leak detection

## Contributing

1. Follow our coding standards
2. Add unit tests for new features
3. Update documentation
4. Include benchmark results

## License

This project is licensed under the MIT License. 
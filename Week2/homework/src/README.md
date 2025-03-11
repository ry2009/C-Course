# Implementation Guidelines

This directory contains skeleton implementations for your Week 2 homework assignment on memory management in quantitative finance. Your task is to implement the missing functionality in these files:

1. `order_book_allocator.cpp` - Implement a custom memory allocator for order book data
2. `market_data_manager.cpp` - Implement a market data subscription system with memory management
3. `time_series_pool.cpp` - Implement a pool-based allocation system for time series data
4. `lock_free_components.cpp` - Implement lock-free data structures for concurrent access

## Implementation Notes

- Look for `TODO` comments in each file for specific implementation requirements
- Focus on memory management strategies discussed in class
- Pay attention to performance considerations, especially cache efficiency and alignment
- For all implementations, consider thread safety and potential concurrency issues
- Use appropriate memory fences and atomic operations where needed

## Testing Your Implementation

You can test your implementation using the provided test files in the `../tests` directory:

```bash
# Build and run the tests
g++ -std=c++17 -o test_order_book ../tests/order_book_allocator_test.cpp order_book_allocator.cpp -lgtest -lpthread
./test_order_book

# Similar commands for other components
```

## Performance Considerations

Your implementation will be evaluated on:

1. Correctness - Does it pass all tests?
2. Memory efficiency - Does it minimize allocations and fragmentation?
3. Speed - Is it optimized for performance?
4. Thread safety - Does it handle concurrent access correctly?

## Submission

Once you have completed the implementation, submit all your `.cpp` files along with a brief report explaining your implementation choices and any optimizations you made. 
# Week 3 Homework: Threading in Quantitative Finance

Welcome to the Week 3 homework assignment on threading in quantitative finance. This assignment builds directly on your work from Weeks 1 and 2, adding multi-threading capabilities to create a more complete trading system. You'll now implement thread-safe components while integrating the sorting algorithms and memory management techniques you've already developed.

## Integration with Previous Weeks

This assignment continues your progression toward a complete trading system:

- **Week 1 (Sorting Algorithms)**: You implemented efficient sorting algorithms for financial data
- **Week 2 (Memory Management)**: You developed custom memory allocators and pools for trading data
- **Week 3 (Threading)**: Now you'll make these components thread-safe and add parallel processing

The goal is to create a cohesive system where your Week 1 sorting algorithms and Week 2 memory management work seamlessly with your new threading capabilities.

## Directory Structure

```
Week3/homework/
├── include/                # Header files
├── src/                    # Source files and starter code
│   ├── market_data_handler.hpp  # Thread-safe market data handler header
│   ├── thread_pool.hpp           # Thread pool implementation header
│   ├── lock_free_queue.hpp       # Lock-free queue implementation header
│   └── ...                       # Implementation files you'll create
├── tests/                  # Test files for your implementation
├── CMakeLists.txt          # Build configuration
├── homework.md             # Homework assignment details
└── README.md               # This file
```

## Getting Started

1. Read the `homework.md` file for a detailed description of the assignment requirements.
2. Review the provided header files in the `src` directory to understand the interfaces you need to implement.
3. **Import your previous work**:
   - Copy your sorting implementations from Week 1 where needed
   - Import your custom allocators and memory pools from Week 2
4. Create implementation files for each component:
   - `market_data_handler.cpp` (using your Week 2 allocator)
   - `thread_pool.cpp` (using your Week 2 memory pool)
   - `lock_free_queue.cpp` (template implementation can be in the header)

## Implementation Notes

### Part 1: Thread-Safe Market Data Handler

- Implement a thread-safe market data handler that can process data from multiple exchanges concurrently
- Focus on proper synchronization while minimizing lock contention
- **Integration point**: Use your Week 2 `OrderBookAllocator` for memory management
- **Integration point**: Apply your Week 1 sorting algorithms to maintain price levels
- Track performance metrics to evaluate your implementation

### Part 2: Thread Pool for Strategy Evaluation

- Implement a thread pool that can execute tasks with different priorities
- Support dynamic resizing based on system load
- Implement work stealing for better load balancing
- **Integration point**: Use your Week 2 memory pool for task allocation
- **Integration point**: Use your Week 1 sorting for strategy prioritization

### Part 3: Lock-Free Data Structures

- Implement a lock-free queue supporting multiple producers and multiple consumers
- Handle the ABA problem correctly
- Compare performance with mutex-based implementations
- **Integration point**: Apply your Week 2 memory management techniques

## Building and Testing

```bash
# Create a build directory
mkdir build && cd build

# Generate build files
cmake ..

# Build the project
cmake --build .

# Run tests
ctest
```

## Integration Testing

In addition to component tests, create integrated tests that demonstrate how your Week 1-3 components work together:

```bash
# Run the integrated system test
./integrated_system_test

# Run performance benchmarks
./integrated_performance_test
```

## Performance Testing

For each component, compare the performance of your implementation with different thread counts and workloads. Include these comparisons in your submission.

Example:
```bash
# Run market data handler performance test
./market_data_handler_perf

# Run thread pool performance test
./thread_pool_perf

# Run lock-free queue performance test
./lock_free_queue_perf
```

## Helpful Resources

1. C++ Standard Library Threading Documentation:
   - [`<thread>`](https://en.cppreference.com/w/cpp/thread/thread)
   - [`<mutex>`](https://en.cppreference.com/w/cpp/thread/mutex)
   - [`<atomic>`](https://en.cppreference.com/w/cpp/atomic/atomic)
   - [`<condition_variable>`](https://en.cppreference.com/w/cpp/thread/condition_variable)

2. Lock-Free Programming References:
   - [Lock-Free Programming with C++ Atomics](https://www.codeproject.com/Articles/1183423/Lock-Free-Programming-with-Cplusplus-Atomics)
   - [Notes on Lock-Free Data Structures](https://www.cs.cmu.edu/~410-s05/lectures/L31_LockFree.pdf)

3. Thread Pool Implementation Guides:
   - [C++11 Thread Pool Implementation](https://github.com/progschj/ThreadPool)
   - [A simple implementation of a work stealing thread pool in C++](https://github.com/taskflow/work-stealing-queue)

4. Integration Resources:
   - Review your Week 1 and Week 2 code for integration points
   - [Memory-efficient data structures in C++](https://en.cppreference.com/w/cpp/memory)
   - [Threading and memory model](https://en.cppreference.com/w/cpp/language/memory_model)

## Submission Guidelines

Your submission should include:

1. Complete implementation files for all required components
2. Unit tests demonstrating correctness
3. Performance benchmarks comparing different approaches
4. A design document explaining your implementation choices
5. Analysis of how you integrated components from all three weeks
6. Analysis of performance characteristics and bottlenecks

Submit your completed assignment by the due date specified in `homework.md`.

## Project Evolution

This assignment completes the foundational components of your trading system. By the end, you'll have:

1. **Week 1**: Efficient algorithms for data processing
2. **Week 2**: Memory-efficient data structures
3. **Week 3**: Thread-safe, concurrent processing capabilities

In future weeks, we'll expand on this foundation to add network communication, order routing, and risk management functionality.

## Questions and Help

If you have questions about the assignment, please post them on the course forum or contact the teaching assistants. 
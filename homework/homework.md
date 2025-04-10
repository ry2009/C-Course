# Week 3 Homework: Threading in Quantitative Finance

## Overview

In this homework assignment, you will extend your trading system from the previous weeks by implementing multi-threaded components. Building on the sorting algorithms from Week 1 and the memory management techniques from Week 2, you'll now add concurrent processing capabilities to handle multiple data streams and parallel calculations.

This assignment connects directly to your previous work:
- Your Week 1 assignment implemented efficient sorting for financial data
- Your Week 2 assignment built memory-efficient structures for order books and trading data
- Now you'll make these components thread-safe and implement parallel processing

## Part 1: Thread-Safe Market Data Handler (40 points)

Implement a thread-safe market data handler that processes incoming market data from multiple exchanges.

### Requirements:

1. **Multi-Source Processing (15 points)**
   - Implement a `MarketDataHandler` class that can simultaneously process data from multiple market data sources
   - Each market data source runs in its own thread
   - Support at least 3 different exchange feeds simultaneously
   - **Integration with Week 2**: Use your custom `OrderBookAllocator` from Week 2 to efficiently manage memory for market data updates

2. **Thread-Safe Updates (15 points)**
   - Ensure thread-safe updates to the order book
   - Implement a suitable synchronization mechanism (mutexes, reader-writer locks, etc.)
   - Minimize lock contention for high-throughput scenarios
   - **Integration with Week 1**: Use your sorting implementation when maintaining sorted price levels in the order book across concurrent updates

3. **Performance Metrics (10 points)**
   - Track and report performance metrics:
     - Average processing latency per exchange
     - Message throughput (updates per second)
     - Lock contention statistics
     - Memory allocation patterns (building on Week 2's monitoring)

## Part 2: Thread Pool for Strategy Evaluation (30 points)

Implement a thread pool system for parallel strategy evaluation that uses your memory management system from Week 2.

### Requirements:

1. **Thread Pool Implementation (15 points)**
   - Create a generic thread pool that can execute arbitrary tasks
   - Support dynamic sizing based on system load
   - Implement work stealing for better load balancing
   - **Integration with Week 2**: Use your custom memory pool implementation from Week 2 to manage memory for task objects

2. **Strategy Execution (15 points)**
   - Use your thread pool to evaluate multiple trading strategies in parallel
   - Implement a priority system for strategies (high, medium, low)
   - Ensure results are collected and aggregated correctly
   - **Integration with Week 1**: Use your sorting implementation to rank and prioritize strategy signals

## Part 3: Lock-Free Data Structures (30 points)

Implement lock-free data structures for high-performance components of the trading system, building on your Week 2 memory management.

### Requirements:

1. **Lock-Free Queue (15 points)**
   - Implement a lock-free queue for inter-thread communication
   - Support multiple producers and multiple consumers
   - Ensure ABA problem is properly addressed
   - **Integration with Week 2**: Use your memory management techniques to handle dynamic memory requirements

2. **Performance Comparison (15 points)**
   - Compare your lock-free implementation with a mutex-based implementation
   - Measure and report throughput, latency, and scalability with different thread counts
   - Analyze the trade-offs between the two approaches
   - Compare memory efficiency metrics from Week 2's monitoring framework

## Bonus Challenge: Thread-Safe Backtesting Framework (20 points)

Extend your system with a thread-safe backtesting framework that can test multiple strategies against historical data in parallel.

### Requirements:

1. **Parallel Backtesting (10 points)**
   - Implement a backtesting framework that can test multiple strategies in parallel
   - Ensure deterministic results regardless of thread execution order
   - Support various time periods and instruments
   - **Integration with Weeks 1 & 2**: Use your sorting algorithms for event processing and your memory management for efficient data handling

2. **Resource Management (10 points)**
   - Implement efficient resource management for memory and CPU
   - Support backpressure mechanisms to prevent resource exhaustion
   - Provide controls to limit resource usage per backtest
   - Build on your Week 2 performance monitoring to track resource usage

## System Integration

This assignment is designed to build on your previous work and create a cohesive trading system. By the end of this assignment, you should have:

1. A thread-safe market data processing system
2. A parallel strategy evaluation framework
3. Efficient lock-free data structures for inter-thread communication
4. Memory-efficient components that work well under concurrent access
5. A complete system that demonstrates the progression from Week 1 (efficient algorithms) through Week 2 (memory management) to Week 3 (concurrent processing)

## Submission Requirements

1. **Code Implementation**:
   - Complete C++ implementation of all required components
   - Updated integration with your Week 1 and Week 2 code
   - Comprehensive unit tests for all functionality
   - CMake build files to compile the project

2. **Performance Analysis**:
   - Detailed performance measurements for all components
   - Comparison of different synchronization strategies
   - Analysis of scaling behavior with different thread counts
   - Memory usage analysis that builds on Week 2's monitoring

3. **Design Document**:
   - Explanation of your design choices
   - Discussion of synchronization mechanisms used
   - Analysis of potential bottlenecks and how you addressed them
   - Description of how you integrated the Week 1-3 components

## Grading Criteria

- **Correctness (40%)**: Thread safety, absence of race conditions and deadlocks
- **Performance (30%)**: Efficiency, scalability, and minimal synchronization overhead
- **Integration (15%)**: How well you integrated Week 1-3 components
- **Design (10%)**: Architecture, code organization, and appropriate use of C++ features
- **Documentation (5%)**: Clear explanations and thorough performance analysis

## Guidelines

- Use C++17 features when appropriate
- Avoid external libraries beyond the standard library and standard threading libraries
- Consider cache effects and memory layout in your implementations
- Pay attention to the proper termination of threads and cleanup of resources
- Include appropriate error handling for thread-related failures
- Build on your existing code from Weeks 1 and 2 rather than starting from scratch

## Due Date

Submit your completed assignment by 11:59 PM on [due date].

## Resources

- Class notes on threading concepts
- Code examples from the lecture
- C++ Standard Library threading documentation
- Your code from Weeks 1 and 2 
# C++ for Quantitative Finance: Course Plan

## Course Overview
This course introduces C++ programming with a focus on applications in quantitative finance. We'll explore why C++ remains the language of choice for high-performance financial systems, and dive into practical examples starting with sorting algorithms.

## Session 1: Sorting Algorithms in Quantitative Finance

### Learning Objectives
- Understand why C++ is critical in quantitative finance
- Learn about different sorting algorithms and their performance characteristics
- Master the use of `std::sort` and understand its internal workings (Introsort)
- Apply sorting algorithms to financial data problems
- Gain hands-on experience with C++ performance optimization

### Schedule (90 minutes)

| Time | Activity | Description |
|------|----------|-------------|
| 0:00-10:00 | Introduction | C++ in finance, course overview |
| 10:00-25:00 | C++ Fundamentals | Key language features for finance |
| 25:00-45:00 | Sorting Algorithms | Algorithm comparison with Manim visualization |
| 45:00-60:00 | `std::sort` Deep Dive | Implementation details and performance |
| 60:00-80:00 | Hands-on Activity | Sorting financial data exercise |
| 80:00-90:00 | Wrap-up & Homework | Assignment and next steps |

### Materials Needed
- Computers with C++ compiler installed (GCC/Clang/MSVC)
- Visual Studio Code or preferred IDE
- Sample financial datasets
- Manim animation (pre-rendered)

## Detailed Lesson Plan

### 1. Introduction (10 minutes)

#### Key Talking Points
- Why C++ is the language of choice in quantitative finance:
  - Performance: Microsecond/nanosecond latency requirements
  - Memory control: Direct memory management for large datasets
  - Deterministic behavior: Predictable execution times
  - Legacy systems: Many financial systems are built in C++
  - Hardware interaction: Direct access to networking hardware for HFT

#### Transcript Snippet
*"Welcome to our C++ for Quantitative Finance course. Today, we'll explore why C++ remains the dominant language in high-frequency trading and other performance-critical financial applications. We'll start with sorting algorithms - a seemingly basic operation that has profound implications for trading system performance."*

### 2. C++ Fundamentals for Finance (15 minutes)

#### Key Concepts to Cover
- Static typing and type safety
- Memory management (stack vs heap)
- Value semantics vs reference semantics
- STL containers and algorithms
- Compilation model and optimization

#### Code Examples
```cpp
// Example: Value vs Reference Semantics
std::vector<double> prices = {100.0, 101.5, 99.75};
auto prices_copy = prices;  // Deep copy (value semantics)
prices_copy[0] = 105.0;     // Original prices unchanged

// Example: STL algorithms
std::sort(prices.begin(), prices.end());
auto max_price = *std::max_element(prices.begin(), prices.end());
```

### 3. Sorting Algorithms in Finance (20 minutes)

#### Algorithm Comparison Table
| Algorithm | Average Case | Worst Case | Memory | Stable | Use Case in Finance |
|-----------|--------------|------------|--------|--------|---------------------|
| Quick Sort | O(n log n) | O(n²) | O(log n) | No | General purpose, but risky for time-critical systems |
| Merge Sort | O(n log n) | O(n log n) | O(n) | Yes | When stability matters (e.g., preserving time priority) |
| Heap Sort | O(n log n) | O(n log n) | O(1) | No | Memory-constrained environments |
| Introsort | O(n log n) | O(n log n) | O(log n) | No | Production trading systems (std::sort) |

#### Manim Animation
- Show visual comparison of algorithms
- Demonstrate how Introsort adapts based on data characteristics
- Highlight performance differences with financial data

### 4. std::sort Deep Dive (15 minutes)

#### Implementation Details
- Hybrid approach (QuickSort + HeapSort + InsertionSort)
- Recursion depth monitoring
- Partition size optimization
- Memory efficiency

#### Performance Benchmarking
```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>

void benchmark_sort(int size) {
    // Generate random data
    std::vector<double> data(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(1.0, 1000.0);
    
    for (auto& val : data) {
        val = dis(gen);
    }
    
    // Time the sort
    auto start = std::chrono::high_resolution_clock::now();
    std::sort(data.begin(), data.end());
    auto end = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "Sorting " << size << " elements took " 
              << duration.count() << " ms" << std::endl;
}

int main() {
    for (int size : {1000, 10000, 100000, 1000000}) {
        benchmark_sort(size);
    }
}
```

### 5. Hands-on Activity (20 minutes)

#### Order Book Sorting Exercise
```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

struct Order {
    std::string symbol;
    double price;
    int quantity;
    bool is_buy;
    long timestamp;
    
    // For debugging
    void print() const {
        std::cout << symbol << " | " 
                  << (is_buy ? "BUY" : "SELL") << " | "
                  << price << " | " 
                  << quantity << std::endl;
    }
};

// TODO: Implement order book sorting function
// Sort buy orders by price (descending)
// Sort sell orders by price (ascending)
// For same price, sort by timestamp (ascending)

int main() {
    // Sample order book data
    std::vector<Order> orders = {
        {"AAPL", 150.25, 100, true, 1623456789},
        {"AAPL", 150.50, 200, false, 1623456790},
        {"AAPL", 150.25, 150, true, 1623456791},
        {"AAPL", 150.00, 100, false, 1623456792},
        {"AAPL", 150.50, 300, true, 1623456793},
    };
    
    // TODO: Sort the order book
    
    // Print sorted order book
    std::cout << "Sorted Order Book:" << std::endl;
    for (const auto& order : orders) {
        order.print();
    }
}
```

### 6. Wrap-up & Homework (10 minutes)

#### Homework Assignment
1. Implement a custom sorting function for financial time series data
2. Compare performance of different sorting algorithms with large datasets
3. Optimize the order book sorting implementation

#### Next Session Preview
- Memory management in C++
- Custom allocators for financial applications
- Lock-free data structures for high-frequency trading

## Additional Resources

### Recommended Reading
- "Effective Modern C++" by Scott Meyers
- "C++ Concurrency in Action" by Anthony Williams
- "Inside the C++ Object Model" by Stanley Lippman

### Online Resources
- CPPReference: https://en.cppreference.com/
- C++ Core Guidelines: https://isocpp.github.io/CppCoreGuidelines/
- Compiler Explorer: https://godbolt.org/

### Financial C++ Libraries
- QuantLib: https://www.quantlib.org/
- FastFlow: http://calvados.di.unipi.it/
- Boost: https://www.boost.org/ 
# C++ Course: Algorithms in Quantitative Finance

Welcome to the C++ course focused on high-performance programming for quantitative finance. This repository contains all the materials, code examples, and exercises for the course.

## Course Overview

This course explores C++ programming techniques essential for building high-performance financial applications, with a particular focus on sorting algorithms and their applications in quantitative finance. Throughout the course, we'll cover:

1. **Sorting algorithms** and their performance characteristics in financial contexts
2. **Memory management** techniques for high-frequency trading systems
3. **Concurrency patterns** for parallel financial calculations
4. **Optimization strategies** specific to financial workloads

## Repository Structure

```
├── Week1/                  # Week 1: Sorting Algorithms in Finance
│   ├── README.md           # Week 1 overview and exercises
│   ├── Homework/           # Homework assignments
│   ├── order_book_sorting_exercise.cpp
│   └── sorting_benchmark.cpp
│
├── Week2/                  # Week 2: Memory Management
│   └── README.md           # Week 2 overview and exercises
│
├── Week3/                  # Week 3: Concurrency and Parallelism
│   └── README.md           # Week 3 overview and exercises
│
├── Resources/              # Shared resources for all weeks
│   ├── Slides/             # Presentation materials
│   │   └── Week1_Presentation.md
│   │
│   ├── Code_Examples/      # Example code and demonstrations
│   │   └── sorting_algorithms_manim.py
│   │
│   └── README.md           # Resources overview
│
└── README.md               # This file
```

## Getting Started

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, or MSVC 19.14+)
- Python 3.7+ (for Manim animations)
- Git

### Setting Up the Environment

1. Clone this repository:
   ```bash
   git clone https://github.com/yourusername/cpp-finance-course.git
   cd cpp-finance-course
   ```

2. For C++ examples:
   ```bash
   # Compile a C++ example
   g++ -std=c++17 -O3 Week1/sorting_benchmark.cpp -o sorting_benchmark
   
   # Run the compiled program
   ./sorting_benchmark
   ```

3. For Manim animations:
   ```bash
   # Create a virtual environment
   python -m venv venv
   source venv/bin/activate  # On Windows: venv\Scripts\activate
   
   # Install dependencies
   pip install manim numpy
   
   # Run an animation
   manim -pqh Resources/Code_Examples/sorting_algorithms_manim.py SortingAlgorithm
   ```

## Course Schedule

### Week 1: Sorting Algorithms in Quantitative Finance
- Introduction to C++ in finance
- Sorting algorithms comparison
- std::sort deep dive
- Order book sorting implementation

### Week 2: Memory Management in C++ for Financial Applications
- Memory models and performance
- Custom allocators for financial data structures
- Memory pools and arenas
- Lock-free data structures

### Week 3: Concurrency and Parallelism in C++ for Financial Applications
- Thread-safe financial data structures
- Parallel algorithms for financial calculations
- SIMD acceleration for vector operations
- Modern C++ concurrency features

## Contributing

This repository is primarily for educational purposes. If you find errors or have suggestions for improvements:

1. Fork the repository
2. Create a new branch for your changes
3. Submit a pull request with a clear description of your additions

## License

All materials in this repository are provided for educational purposes. Please respect the intellectual property rights and do not redistribute without permission.

## Acknowledgments

- Thanks to all contributors and students who help improve these materials
- Special thanks to the C++ community for developing such a powerful language for financial applications

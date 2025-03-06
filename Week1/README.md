# Week 1: Sorting Algorithms in Quantitative Finance

## Overview
This week introduces sorting algorithms in C++ with a focus on their applications in quantitative finance. We'll explore why `std::sort` (Introsort) is the preferred choice for production financial systems.

## Learning Objectives
- Understand why C++ is critical in quantitative finance
- Learn about different sorting algorithms and their performance characteristics
- Master the use of `std::sort` and understand its internal workings (Introsort)
- Apply sorting algorithms to financial data problems
- Gain hands-on experience with C++ performance optimization

## Materials
- [Presentation Transcript](../Resources/Slides/Week1_Presentation.md)
- [Course Plan](../Resources/Slides/Week1_Course_Plan.md)

## Exercises
- [Order Book Sorting Exercise](order_book_sorting_exercise.cpp)
- [Sorting Benchmark](sorting_benchmark.cpp)

## Homework Assignment
1. Implement a custom sorting function for financial time series data that sorts by date but can efficiently find data points within a specific date range.
2. Compare the performance of `std::sort` with at least one other sorting algorithm (e.g., QuickSort, MergeSort) using a large dataset (at least 1 million elements).
3. Optimize the order book sorting implementation we worked on today, considering the case where new orders are continuously being added.

## Submission Guidelines
- Create a fork of this repository
- Complete the homework in the `Week1/Homework` directory
- Submit a pull request with your solutions by [due date]

## Additional Resources
- [C++ Reference](https://en.cppreference.com/)
- [Sorting Algorithm Visualizations](https://visualgo.net/en/sorting)
- [QuantLib Documentation](https://www.quantlib.org/docs.shtml) 
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <functional>
#include <iomanip>
#include <string>

// Custom implementation of QuickSort for comparison
template<typename T, typename Compare>
void quick_sort(typename std::vector<T>::iterator begin, typename std::vector<T>::iterator end, Compare comp) {
    if (begin >= end - 1) return;
    
    auto pivot = *(begin + (end - begin) / 2);
    auto middle1 = std::partition(begin, end, [&](const T& elem) {
        return comp(elem, pivot);
    });
    
    auto middle2 = std::partition(middle1, end, [&](const T& elem) {
        return !comp(pivot, elem);
    });
    
    quick_sort<T, Compare>(begin, middle1, comp);
    quick_sort<T, Compare>(middle2, end, comp);
}

// Custom implementation of MergeSort for comparison
template<typename T, typename Compare>
void merge_sort(typename std::vector<T>::iterator begin, typename std::vector<T>::iterator end, Compare comp) {
    if (end - begin <= 1) return;
    
    auto middle = begin + (end - begin) / 2;
    merge_sort<T, Compare>(begin, middle, comp);
    merge_sort<T, Compare>(middle, end, comp);
    
    std::vector<T> buffer(end - begin);
    std::merge(begin, middle, middle, end, buffer.begin(), comp);
    std::copy(buffer.begin(), buffer.end(), begin);
}

// Financial data structure
struct StockPrice {
    std::string symbol;
    double price;
    long timestamp;
    
    // For random generation
    static StockPrice random(std::mt19937& gen) {
        static std::vector<std::string> symbols = {"AAPL", "MSFT", "GOOGL", "AMZN", "FB", "TSLA", "JPM", "V", "JNJ", "WMT"};
        static std::uniform_int_distribution<> symbol_dist(0, symbols.size() - 1);
        static std::uniform_real_distribution<> price_dist(50.0, 1000.0);
        static std::uniform_int_distribution<long> time_dist(1600000000, 1630000000);
        
        return {
            symbols[symbol_dist(gen)],
            price_dist(gen),
            time_dist(gen)
        };
    }
};

// Generate a dataset of random stock prices
std::vector<StockPrice> generate_dataset(size_t size) {
    std::random_device rd;
    std::mt19937 gen(rd());
    
    std::vector<StockPrice> data(size);
    for (auto& item : data) {
        item = StockPrice::random(gen);
    }
    
    return data;
}

// Generate a partially sorted dataset (common in financial time series)
std::vector<StockPrice> generate_partially_sorted_dataset(size_t size, double sorted_percentage = 0.7) {
    auto data = generate_dataset(size);
    
    // Sort a portion of the data
    size_t sorted_size = static_cast<size_t>(size * sorted_percentage);
    std::sort(data.begin(), data.begin() + sorted_size, 
              [](const StockPrice& a, const StockPrice& b) {
                  return a.timestamp < b.timestamp;
              });
    
    return data;
}

// Benchmark a sorting algorithm
template<typename SortFunc>
double benchmark_sort(std::vector<StockPrice> data, SortFunc sort_func, const std::string& algorithm_name) {
    auto start = std::chrono::high_resolution_clock::now();
    
    sort_func(data);
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    
    std::cout << std::left << std::setw(15) << algorithm_name 
              << std::setw(12) << data.size() 
              << std::fixed << std::setprecision(2) << duration.count() << " ms" 
              << std::endl;
    
    return duration.count();
}

int main() {
    std::cout << "Sorting Algorithm Benchmark for Financial Data" << std::endl;
    std::cout << "=============================================" << std::endl;
    
    std::vector<size_t> dataset_sizes = {1000, 10000, 100000, 1000000};
    
    std::cout << std::left << std::setw(15) << "Algorithm" 
              << std::setw(12) << "Data Size" 
              << "Time" << std::endl;
    std::cout << "---------------------------------------------" << std::endl;
    
    // Test with random data
    std::cout << "\nRandom Data:" << std::endl;
    for (auto size : dataset_sizes) {
        auto data = generate_dataset(size);
        
        // std::sort
        auto std_sort_data = data;
        benchmark_sort(std_sort_data, [](std::vector<StockPrice>& d) {
            std::sort(d.begin(), d.end(), [](const StockPrice& a, const StockPrice& b) {
                return a.price < b.price;
            });
        }, "std::sort");
        
        // Custom QuickSort
        if (size <= 100000) {  // Skip large datasets for slower algorithms
            auto quick_sort_data = data;
            benchmark_sort(quick_sort_data, [](std::vector<StockPrice>& d) {
                quick_sort<StockPrice>(d.begin(), d.end(), [](const StockPrice& a, const StockPrice& b) {
                    return a.price < b.price;
                });
            }, "QuickSort");
        }
        
        // Custom MergeSort
        if (size <= 100000) {  // Skip large datasets for slower algorithms
            auto merge_sort_data = data;
            benchmark_sort(merge_sort_data, [](std::vector<StockPrice>& d) {
                merge_sort<StockPrice>(d.begin(), d.end(), [](const StockPrice& a, const StockPrice& b) {
                    return a.price < b.price;
                });
            }, "MergeSort");
        }
        
        std::cout << "---------------------------------------------" << std::endl;
    }
    
    // Test with partially sorted data (common in financial time series)
    std::cout << "\nPartially Sorted Data (70% pre-sorted):" << std::endl;
    for (auto size : dataset_sizes) {
        auto data = generate_partially_sorted_dataset(size);
        
        // std::sort
        auto std_sort_data = data;
        benchmark_sort(std_sort_data, [](std::vector<StockPrice>& d) {
            std::sort(d.begin(), d.end(), [](const StockPrice& a, const StockPrice& b) {
                return a.timestamp < b.timestamp;
            });
        }, "std::sort");
        
        // Custom QuickSort
        if (size <= 100000) {  // Skip large datasets for slower algorithms
            auto quick_sort_data = data;
            benchmark_sort(quick_sort_data, [](std::vector<StockPrice>& d) {
                quick_sort<StockPrice>(d.begin(), d.end(), [](const StockPrice& a, const StockPrice& b) {
                    return a.timestamp < b.timestamp;
                });
            }, "QuickSort");
        }
        
        // Custom MergeSort
        if (size <= 100000) {  // Skip large datasets for slower algorithms
            auto merge_sort_data = data;
            benchmark_sort(merge_sort_data, [](std::vector<StockPrice>& d) {
                merge_sort<StockPrice>(d.begin(), d.end(), [](const StockPrice& a, const StockPrice& b) {
                    return a.timestamp < b.timestamp;
                });
            }, "MergeSort");
        }
        
        std::cout << "---------------------------------------------" << std::endl;
    }
    
    return 0;
}

// Compile with:
// g++ -std=c++17 -O3 sorting_benchmark.cpp -o sorting_benchmark 
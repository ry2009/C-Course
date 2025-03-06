#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <chrono>
#include <iomanip>

/**
 * Order struct representing a market order in an order book
 */
struct Order {
    std::string symbol;
    double price;
    int quantity;
    bool is_buy;
    long timestamp;
    
    // For debugging
    void print() const {
        std::cout << std::left 
                  << std::setw(6) << symbol << " | " 
                  << std::setw(4) << (is_buy ? "BUY" : "SELL") << " | "
                  << std::setw(8) << std::fixed << std::setprecision(2) << price << " | " 
                  << std::setw(6) << quantity << " | "
                  << timestamp
                  << std::endl;
    }
};

/**
 * Exercise: Implement order book sorting
 * 
 * Requirements:
 * 1. Buy orders should be sorted by price in descending order (highest price first)
 * 2. Sell orders should be sorted by price in ascending order (lowest price first)
 * 3. Orders with the same price should be sorted by timestamp (earliest first)
 */
void sort_order_book(std::vector<Order>& orders) {
    // TODO: Implement the sorting logic using std::sort
    // Hint: Use a custom comparator function or lambda
    
    // Your code here:
    
}

int main() {
    // Sample order book data
    std::vector<Order> orders = {
        {"AAPL", 150.25, 100, true, 1623456789},
        {"AAPL", 150.50, 200, false, 1623456790},
        {"AAPL", 150.25, 150, true, 1623456791},
        {"AAPL", 150.00, 100, false, 1623456792},
        {"AAPL", 150.50, 300, true, 1623456793},
        {"AAPL", 149.75, 200, true, 1623456794},
        {"AAPL", 150.25, 100, false, 1623456795},
        {"AAPL", 150.00, 250, true, 1623456796},
        {"AAPL", 150.50, 150, false, 1623456797},
    };
    
    // Print original order book
    std::cout << "Original Order Book:" << std::endl;
    std::cout << "=============================" << std::endl;
    std::cout << std::left 
              << std::setw(6) << "Symbol" << " | " 
              << std::setw(4) << "Side" << " | "
              << std::setw(8) << "Price" << " | " 
              << std::setw(6) << "Qty" << " | "
              << "Timestamp"
              << std::endl;
    std::cout << "=============================" << std::endl;
    
    for (const auto& order : orders) {
        order.print();
    }
    
    // Sort the order book
    sort_order_book(orders);
    
    // Print sorted order book
    std::cout << "\nSorted Order Book:" << std::endl;
    std::cout << "=============================" << std::endl;
    std::cout << std::left 
              << std::setw(6) << "Symbol" << " | " 
              << std::setw(4) << "Side" << " | "
              << std::setw(8) << "Price" << " | " 
              << std::setw(6) << "Qty" << " | "
              << "Timestamp"
              << std::endl;
    std::cout << "=============================" << std::endl;
    
    for (const auto& order : orders) {
        order.print();
    }
    
    return 0;
}

/**
 * BONUS EXERCISE:
 * 
 * 1. Measure the performance of your sorting function with a larger dataset
 * 2. Implement an alternative approach that partitions the orders into buy and sell groups first,
 *    then sorts each group separately. Compare the performance.
 * 3. Optimize for the case where new orders are continuously being added (mostly in timestamp order)
 */ 
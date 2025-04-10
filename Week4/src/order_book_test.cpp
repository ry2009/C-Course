#include "order_book_processor.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <random>

// Helper function to get current timestamp in milliseconds
uint64_t getCurrentTimestamp() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

// Helper to create a market data update with sample data
MarketDataUpdate createSampleMarketData(const std::string& symbol, 
                                        double basePrice, 
                                        uint64_t timestamp) {
    MarketDataUpdate update;
    update.symbol = symbol;
    update.timestamp = timestamp;
    update.lastTradePrice = basePrice;
    
    // Create a few levels of bids (sorted by price descending)
    for (int i = 0; i < 5; i++) {
        OrderBookEntry bid(basePrice - (i * 0.01), 100.0 + (i * 50), OrderSide::BUY, 5 - i);
        update.bids.push_back(bid);
    }
    
    // Create a few levels of asks (sorted by price ascending)
    for (int i = 0; i < 5; i++) {
        OrderBookEntry ask(basePrice + ((i + 1) * 0.01), 100.0 + (i * 30), OrderSide::SELL, i + 1);
        update.asks.push_back(ask);
    }
    
    return update;
}

// Helper to create a sample order
Order createSampleOrder(const std::string& symbol, 
                        OrderType type, 
                        OrderSide side, 
                        double price, 
                        double quantity) {
    static int orderId = 1000;
    
    Order order;
    order.orderId = "ORD" + std::to_string(orderId++);
    order.symbol = symbol;
    order.type = type;
    order.side = side;
    order.price = price;
    order.quantity = quantity;
    order.timestamp = getCurrentTimestamp();
    
    return order;
}

int main() {
    // Create an order book processor
    auto processor = createOrderBookProcessor();
    
    const std::string symbol = "AAPL";
    double basePrice = 150.0;
    
    std::cout << "===== Order Book Processor Demonstration =====" << std::endl;
    
    // Initialize with market data
    auto marketData = createSampleMarketData(symbol, basePrice, getCurrentTimestamp());
    processor->processMarketDataUpdate(marketData);
    
    // Display initial order book
    std::cout << "\nInitial Order Book:" << std::endl;
    std::cout << processor->getOrderBookSnapshot(symbol, 5) << std::endl;
    
    // Process some limit orders
    std::cout << "\nProcessing limit orders..." << std::endl;
    
    // Add a buy limit order
    Order buyOrder = createSampleOrder(symbol, OrderType::LIMIT, OrderSide::BUY, 
                                       basePrice - 0.02, 200);
    processor->processOrder(buyOrder);
    
    // Add a sell limit order
    Order sellOrder = createSampleOrder(symbol, OrderType::LIMIT, OrderSide::SELL, 
                                        basePrice + 0.03, 150);
    processor->processOrder(sellOrder);
    
    // Display order book after limit orders
    std::cout << "\nOrder Book after limit orders:" << std::endl;
    std::cout << processor->getOrderBookSnapshot(symbol, 5) << std::endl;
    
    // Process a market order
    std::cout << "\nProcessing market order..." << std::endl;
    Order marketOrder = createSampleOrder(symbol, OrderType::MARKET, OrderSide::BUY, 
                                         0.0, 120);
    processor->processOrder(marketOrder);
    
    // Display order book after market order
    std::cout << "\nOrder Book after market order:" << std::endl;
    std::cout << processor->getOrderBookSnapshot(symbol, 5) << std::endl;
    
    // Calculate and display some microstructure metrics
    std::cout << "\n===== Market Microstructure Metrics =====" << std::endl;
    std::cout << "Current spread: " << processor->getSpread(symbol) << std::endl;
    std::cout << "Volume imbalance: " << processor->getVolumeImbalance(symbol) << std::endl;
    std::cout << "Mid price: " << processor->getMidPrice(symbol) << std::endl;
    std::cout << "Micro price: " << processor->getMicroPrice(symbol) << std::endl;
    
    // Simulate market data updates
    std::cout << "\n===== Simulating Market Changes =====" << std::endl;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> priceChanges(0.0, 0.05);
    
    for (int i = 0; i < 10; i++) {
        // Sleep for 100ms to simulate time passing
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Update the base price with some random changes
        basePrice += priceChanges(gen);
        
        // Create and process new market data
        auto newData = createSampleMarketData(symbol, basePrice, getCurrentTimestamp());
        processor->processMarketDataUpdate(newData);
        
        std::cout << "\nUpdate " << (i+1) << " - New Mid Price: " 
                  << processor->getMidPrice(symbol) << std::endl;
    }
    
    // Display final order book
    std::cout << "\nFinal Order Book:" << std::endl;
    std::cout << processor->getOrderBookSnapshot(symbol, 5) << std::endl;
    
    // Calculate advanced metrics
    std::cout << "\n===== Advanced Market Analysis =====" << std::endl;
    
    // Order flow toxicity analysis
    auto toxicityMetrics = processor->analyzeOrderFlowToxicity(symbol, 100);
    std::cout << "Order Flow Toxicity Metrics:" << std::endl;
    for (const auto& [metric, value] : toxicityMetrics) {
        std::cout << "  " << metric << ": " << value << std::endl;
    }
    
    // Calculate effective spread
    auto effectiveSpread = processor->calculateEffectiveSpread(symbol, 100);
    std::cout << "Effective Spread (buy side): " << effectiveSpread.first << std::endl;
    std::cout << "Effective Spread (sell side): " << effectiveSpread.second << std::endl;
    
    // Historical volatility
    std::cout << "Historical volatility: " << processor->getHistoricalVol(symbol, 100) << std::endl;
    
    return 0;
} 
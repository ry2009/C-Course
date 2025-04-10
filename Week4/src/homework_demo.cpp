#include "order_book_processor.h"
#include "risk_calculator.h"
#include "trading_strategy.h"
#include "message_bus.h"
#include "system_monitor.h"
#include <iostream>
#include <chrono>
#include <thread>

// Helper function to print order book state
void printOrderBook(const OrderBookProcessor& processor, const std::string& symbol) {
    std::cout << "\nOrder Book for " << symbol << ":" << std::endl;
    std::cout << processor.getOrderBookSnapshot(symbol, 5) << std::endl;
}

// Helper function to print risk metrics
void printRiskMetrics(const RiskCalculator& risk, const std::string& symbol) {
    std::cout << "\nRisk Metrics for " << symbol << ":" << std::endl;
    std::cout << "Position Risk: " << risk.calculateRisk(symbol) << std::endl;
    Position pos = risk.getPosition(symbol);
    std::cout << "Current Position: " << pos.quantity << " (Max: " << pos.maxPosition << ")" << std::endl;
}

// Helper function to print strategy metrics
void printStrategyMetrics(const TradingStrategy& strategy, const std::string& symbol) {
    std::cout << "\nStrategy Metrics for " << symbol << ":" << std::endl;
    std::cout << "Optimal Order Size: " << strategy.calculateOptimalSize(symbol) << std::endl;
    std::cout << "Current Spread: " << strategy.calculateSpread(symbol) << std::endl;
}

int main() {
    std::cout << "===== Trading System Demonstration =====" << std::endl;
    
    // Create system components
    OrderBookProcessor processor;
    RiskCalculator risk;
    TradingStrategy strategy;
    MessageBus bus;
    SystemMonitor monitor;
    
    // Initialize components
    bus.start();
    monitor.start();
    
    // Subscribe components to messages
    bus.subscribe("market_data", [&](const Message& msg) {
        MarketDataUpdate update = msg.getData<MarketDataUpdate>();
        processor.processMarketDataUpdate(update);
        strategy.updateMarketConditions(update);
    });
    
    bus.subscribe("order", [&](const Message& msg) {
        Order order = msg.getData<Order>();
        Position position = risk.getPosition(order.symbol);
        if (risk.validateOrder(order, position)) {
            processor.processOrder(order);
            position.quantity += (order.side == OrderSide::BUY ? order.quantity : -order.quantity);
            risk.updatePosition(position);
        }
    });
    
    const std::string symbol = "AAPL";
    double basePrice = 150.0;
    
    // Step 1: Initial Market Data
    std::cout << "\nStep 1: Processing Initial Market Data" << std::endl;
    MarketDataUpdate initialData = createSampleMarketData(symbol, basePrice);
    bus.publish(Message("market_data", &initialData));
    
    printOrderBook(processor, symbol);
    printRiskMetrics(risk, symbol);
    printStrategyMetrics(strategy, symbol);
    
    // Step 2: Process Limit Orders
    std::cout << "\nStep 2: Processing Limit Orders" << std::endl;
    Order buyOrder = createSampleOrder(symbol, OrderType::LIMIT, OrderSide::BUY, basePrice - 0.01, 100);
    Order sellOrder = createSampleOrder(symbol, OrderType::LIMIT, OrderSide::SELL, basePrice + 0.01, 100);
    
    bus.publish(Message("order", &buyOrder));
    bus.publish(Message("order", &sellOrder));
    
    printOrderBook(processor, symbol);
    printRiskMetrics(risk, symbol);
    
    // Step 3: Process Market Order
    std::cout << "\nStep 3: Processing Market Order" << std::endl;
    Order marketOrder = createSampleOrder(symbol, OrderType::MARKET, OrderSide::BUY, 0.0, 50);
    bus.publish(Message("order", &marketOrder));
    
    printOrderBook(processor, symbol);
    printRiskMetrics(risk, symbol);
    
    // Step 4: Simulate Market Changes
    std::cout << "\nStep 4: Simulating Market Changes" << std::endl;
    for (int i = 0; i < 5; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        basePrice += 0.01;
        MarketDataUpdate update = createSampleMarketData(symbol, basePrice);
        bus.publish(Message("market_data", &update));
        
        std::cout << "\nUpdate " << (i+1) << " - New Price: " << basePrice << std::endl;
        printOrderBook(processor, symbol);
    }
    
    // Step 5: Generate Strategy Orders
    std::cout << "\nStep 5: Generating Strategy Orders" << std::endl;
    Order strategyOrder = strategy.generateOrder(initialData);
    bus.publish(Message("order", &strategyOrder));
    
    printOrderBook(processor, symbol);
    printRiskMetrics(risk, symbol);
    printStrategyMetrics(strategy, symbol);
    
    // Step 6: Check System Health
    std::cout << "\nStep 6: System Health Check" << std::endl;
    std::cout << "Order Book Processor: " << (monitor.checkComponentHealth("order_book_processor") ? "Healthy" : "Unhealthy") << std::endl;
    std::cout << "Risk Calculator: " << (monitor.checkComponentHealth("risk_calculator") ? "Healthy" : "Unhealthy") << std::endl;
    std::cout << "Trading Strategy: " << (monitor.checkComponentHealth("trading_strategy") ? "Healthy" : "Unhealthy") << std::endl;
    
    // Cleanup
    bus.stop();
    monitor.stop();
    
    return 0;
} 
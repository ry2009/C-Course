#include "order_book_processor.h"
#include "risk_calculator.h"
#include "trading_strategy.h"
#include "message_bus.h"
#include "system_monitor.h"
#include <gtest/gtest.h>
#include <chrono>
#include <thread>

// Helper functions
MarketDataUpdate createSampleMarketData(const std::string& symbol, double basePrice) {
    MarketDataUpdate update;
    update.symbol = symbol;
    update.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    update.lastTradePrice = basePrice;
    
    // Create sample bids
    for (int i = 0; i < 5; i++) {
        OrderBookEntry bid(basePrice - (i * 0.01), 100.0 + (i * 50), OrderSide::BUY, 5 - i);
        update.bids.push_back(bid);
    }
    
    // Create sample asks
    for (int i = 0; i < 5; i++) {
        OrderBookEntry ask(basePrice + ((i + 1) * 0.01), 100.0 + (i * 30), OrderSide::SELL, i + 1);
        update.asks.push_back(ask);
    }
    
    return update;
}

Order createSampleOrder(const std::string& symbol, OrderType type, OrderSide side, double price, double quantity) {
    static int orderId = 1000;
    
    Order order;
    order.orderId = "ORD" + std::to_string(orderId++);
    order.symbol = symbol;
    order.type = type;
    order.side = side;
    order.price = price;
    order.quantity = quantity;
    order.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    
    return order;
}

// Unit Tests
TEST(OrderBookProcessor, ProcessLimitOrder) {
    OrderBookProcessor processor;
    Order order = createSampleOrder("AAPL", OrderType::LIMIT, OrderSide::BUY, 150.0, 100);
    processor.processOrder(order);
    
    EXPECT_NEAR(processor.getSpread("AAPL"), 0.01, 0.0001);
    EXPECT_NEAR(processor.getVolumeImbalance("AAPL"), 0.0, 0.0001);
}

TEST(RiskCalculator, ValidateOrder) {
    RiskCalculator risk;
    Order order = createSampleOrder("AAPL", OrderType::LIMIT, OrderSide::BUY, 150.0, 1000);
    Position position;
    position.symbol = "AAPL";
    position.quantity = 500;
    position.maxPosition = 2000;
    
    EXPECT_TRUE(risk.validateOrder(order, position));
    
    order.quantity = 2000;
    EXPECT_FALSE(risk.validateOrder(order, position));
}

TEST(TradingStrategy, GenerateOrder) {
    TradingStrategy strategy;
    MarketDataUpdate update = createSampleMarketData("AAPL", 150.0);
    strategy.updateMarketConditions(update);
    
    Order order = strategy.generateOrder(update);
    EXPECT_GT(order.quantity, 0);
    EXPECT_TRUE(order.price > 0);
}

// Integration Tests
TEST(SystemIntegration, OrderFlow) {
    OrderBookProcessor processor;
    RiskCalculator risk;
    TradingStrategy strategy;
    MessageBus bus;
    
    // Subscribe components to messages
    bus.subscribe("market_data", [&](const Message& msg) {
        MarketDataUpdate update = msg.getData<MarketDataUpdate>();
        processor.processMarketDataUpdate(update);
        strategy.updateMarketConditions(update);
    });
    
    bus.subscribe("order", [&](const Message& msg) {
        Order order = msg.getData<Order>();
        Position position;
        if (risk.validateOrder(order, position)) {
            processor.processOrder(order);
        }
    });
    
    // Simulate market data update
    MarketDataUpdate update = createSampleMarketData("AAPL", 150.0);
    bus.publish(Message("market_data", update));
    
    // Generate and process order
    Order order = strategy.generateOrder(update);
    bus.publish(Message("order", order));
    
    // Verify order was processed
    EXPECT_NEAR(processor.getSpread("AAPL"), 0.01, 0.0001);
}

// Performance Tests
TEST(Performance, OrderProcessingLatency) {
    OrderBookProcessor processor;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 1000; i++) {
        Order order = createSampleOrder("AAPL", OrderType::LIMIT, OrderSide::BUY, 150.0, 100);
        processor.processOrder(order);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    EXPECT_LT(duration.count(), 1000000); // Less than 1 second for 1000 orders
}

TEST(Performance, SystemThroughput) {
    OrderBookProcessor processor;
    RiskCalculator risk;
    TradingStrategy strategy;
    MessageBus bus;
    
    auto start = std::chrono::high_resolution_clock::now();
    int ordersProcessed = 0;
    
    // Simulate high throughput
    for (int i = 0; i < 1000; i++) {
        MarketDataUpdate update = createSampleMarketData("AAPL", 150.0 + (i * 0.01));
        bus.publish(Message("market_data", update));
        
        Order order = strategy.generateOrder(update);
        Position position;
        if (risk.validateOrder(order, position)) {
            processor.processOrder(order);
            ordersProcessed++;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    
    EXPECT_GT(ordersProcessed, 0);
    EXPECT_LT(duration.count(), 5); // Should process in less than 5 seconds
}

// System Reliability Tests
TEST(SystemReliability, ComponentHealth) {
    SystemMonitor monitor;
    OrderBookProcessor processor;
    RiskCalculator risk;
    TradingStrategy strategy;
    
    EXPECT_TRUE(monitor.checkComponentHealth("order_book_processor"));
    EXPECT_TRUE(monitor.checkComponentHealth("risk_calculator"));
    EXPECT_TRUE(monitor.checkComponentHealth("trading_strategy"));
}

TEST(SystemReliability, ErrorHandling) {
    OrderBookProcessor processor;
    MarketDataUpdate invalidUpdate;
    invalidUpdate.symbol = "";
    
    // Should handle invalid data gracefully
    EXPECT_NO_THROW(processor.processMarketDataUpdate(invalidUpdate));
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 
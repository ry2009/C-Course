# Distributed Systems in Finance: Instructor Solution

## Overview
This solution demonstrates a simplified trading system with three main components: Order Book Processor, Risk Calculator, and Trading Strategy. The system shows basic distributed system concepts while maintaining simplicity.

## System Architecture

```
+----------------+     +----------------+     +----------------+
| Order Book     |     | Risk          |     | Trading        |
| Processor      |<--->| Calculator    |<--->| Strategy      |
+----------------+     +----------------+     +----------------+
```

## Component Implementation

### 1. Order Book Processor

The Order Book Processor handles order management and matching. Key features:
- Price-time priority matching
- Order book state tracking
- Market metrics calculation

```cpp
class OrderBookProcessor {
public:
    void processOrder(const Order& order);
    void processMarketDataUpdate(const MarketDataUpdate& update);
    double getSpread(const std::string& symbol) const;
    double getVolumeImbalance(const std::string& symbol) const;
    // ... other methods
};
```

### 2. Risk Calculator

The Risk Calculator validates orders and manages risk:
- Position limit checks
- Basic risk metrics
- Circuit breaker implementation

```cpp
class RiskCalculator {
public:
    bool validateOrder(const Order& order, const Position& position);
    double calculateRisk(const std::string& symbol);
    bool checkCircuitBreaker(const MarketDataUpdate& update);
    // ... other methods
};
```

### 3. Trading Strategy

A simple market-making strategy:
- Optimal order size calculation
- Market condition monitoring
- Basic order generation

```cpp
class TradingStrategy {
public:
    Order generateOrder(const MarketDataUpdate& update);
    double calculateOptimalSize(const std::string& symbol);
    void updateMarketConditions(const MarketDataUpdate& update);
    // ... other methods
};
```

## System Integration

### Component Communication

Components communicate through a simple message bus:

```cpp
class MessageBus {
public:
    void publish(const Message& msg);
    void subscribe(const std::string& topic, MessageHandler handler);
    // ... other methods
};
```

### System Reliability

Basic health checks and error handling:

```cpp
class SystemMonitor {
public:
    bool checkComponentHealth(const std::string& component);
    void handleComponentFailure(const std::string& component);
    void logSystemEvent(const SystemEvent& event);
    // ... other methods
};
```

## Testing

### Unit Tests

Example test for Order Book Processor:

```cpp
TEST(OrderBookProcessor, ProcessLimitOrder) {
    OrderBookProcessor processor;
    Order order = createSampleOrder("AAPL", OrderType::LIMIT, OrderSide::BUY, 150.0, 100);
    processor.processOrder(order);
    EXPECT_EQ(processor.getOrderBookSnapshot("AAPL", 1), expectedSnapshot);
}
```

### Integration Tests

Example test for component interaction:

```cpp
TEST(SystemIntegration, OrderFlow) {
    OrderBookProcessor processor;
    RiskCalculator risk;
    TradingStrategy strategy;
    
    MarketDataUpdate update = createSampleMarketData("AAPL", 150.0);
    processor.processMarketDataUpdate(update);
    
    Order order = strategy.generateOrder(update);
    if (risk.validateOrder(order, currentPosition)) {
        processor.processOrder(order);
    }
    
    EXPECT_TRUE(processor.getOrderBookSnapshot("AAPL", 1).contains(order));
}
```

### Performance Testing

Basic performance measurement:

```cpp
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
```

## Running the Solution

1. Build the project:
```bash
mkdir build
cd build
cmake ..
make
```

2. Run the tests:
```bash
./order_book_test
./risk_calculator_test
./trading_strategy_test
./integration_test
./performance_test
```

3. Run the main system:
```bash
./trading_system
```

## Key Design Decisions

1. **Simplicity First**: Focused on core functionality while maintaining clean design
2. **Modular Components**: Each component has clear responsibilities
3. **Basic Reliability**: Simple health checks and error handling
4. **Test Coverage**: Comprehensive tests for each component and integration

## Performance Considerations

1. **Order Processing**: Optimized for typical trading volumes
2. **Memory Usage**: Efficient data structures for order book
3. **Latency**: Minimized processing time for critical operations

## Next Steps

1. Add more sophisticated risk calculations
2. Implement additional trading strategies
3. Enhance system monitoring
4. Add more comprehensive performance testing 
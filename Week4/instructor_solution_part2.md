# Instructor Solution Guide: Part 2 - Data Structure Implementation

## Overview
This solution guide provides reference implementations for the core data structures required in Part 2 of the homework assignment. These implementations demonstrate efficient approaches to building the foundational components of the trade-ngin system.

## 1. Order Book Implementation

A high-performance order book should support fast insertions, deletions, modifications, and top-of-book queries. Here is a recommended implementation:

```cpp
#include <map>
#include <unordered_map>
#include <list>
#include <string>
#include <chrono>
#include <iostream>
#include <memory>

// Price in cents to avoid floating point issues
using Price = int64_t;
using Quantity = int64_t;
using OrderId = std::string;
using Timestamp = int64_t;

enum class Side { BUY, SELL };

struct Order {
    OrderId id;
    Price price;
    Quantity quantity;
    Side side;
    Timestamp timestamp;
    
    // For priority queue
    bool operator<(const Order& other) const {
        if (price == other.price)
            return timestamp > other.timestamp; // Later orders have less priority
        
        if (side == Side::BUY)
            return price < other.price; // Higher buy prices have priority
        else
            return price > other.price; // Lower sell prices have priority
    }
};

// A price level in the order book
class PriceLevel {
private:
    Price price_;
    Quantity totalQuantity_;
    std::list<Order> orders_; // Orders at this price level, time-ordered
    
public:
    PriceLevel(Price price) : price_(price), totalQuantity_(0) {}
    
    Price getPrice() const { return price_; }
    Quantity getTotalQuantity() const { return totalQuantity_; }
    
    // Add order to this price level
    void addOrder(const Order& order) {
        orders_.push_back(order);
        totalQuantity_ += order.quantity;
    }
    
    // Remove order from this price level
    bool removeOrder(const OrderId& orderId) {
        for (auto it = orders_.begin(); it != orders_.end(); ++it) {
            if (it->id == orderId) {
                totalQuantity_ -= it->quantity;
                orders_.erase(it);
                return true;
            }
        }
        return false;
    }
    
    // Modify order quantity
    bool modifyOrderQuantity(const OrderId& orderId, Quantity newQuantity) {
        for (auto& order : orders_) {
            if (order.id == orderId) {
                totalQuantity_ -= order.quantity;
                totalQuantity_ += newQuantity;
                order.quantity = newQuantity;
                return true;
            }
        }
        return false;
    }
    
    // Get the first order (oldest at this price)
    const Order* getFirstOrder() const {
        if (orders_.empty()) return nullptr;
        return &orders_.front();
    }
    
    // Remove the first order and return a copy
    Order popFirstOrder() {
        if (orders_.empty()) throw std::runtime_error("No orders at this price level");
        
        Order first = orders_.front();
        orders_.pop_front();
        totalQuantity_ -= first.quantity;
        return first;
    }
    
    bool isEmpty() const { return orders_.empty(); }
};

class OrderBook {
private:
    std::string symbol_;
    
    // Price level maps, using std::map for ordering
    std::map<Price, PriceLevel, std::greater<Price>> buyLevels_;  // Highest price first
    std::map<Price, PriceLevel, std::less<Price>> sellLevels_;    // Lowest price first
    
    // Quick access to orders by ID
    std::unordered_map<OrderId, std::pair<Side, Price>> orderLocations_;
    
public:
    OrderBook(const std::string& symbol) : symbol_(symbol) {}
    
    // Add a new order to the book
    bool addOrder(const Order& order) {
        // Check if order ID already exists
        if (orderLocations_.find(order.id) != orderLocations_.end()) {
            return false;
        }
        
        // Add to appropriate side
        if (order.side == Side::BUY) {
            auto& level = buyLevels_[order.price];
            if (level.getPrice() == 0) { // New price level
                level = PriceLevel(order.price);
            }
            level.addOrder(order);
        } else {
            auto& level = sellLevels_[order.price];
            if (level.getPrice() == 0) { // New price level
                level = PriceLevel(order.price);
            }
            level.addOrder(order);
        }
        
        // Update order location map
        orderLocations_[order.id] = {order.side, order.price};
        
        // Check for matches after adding
        matchOrders();
        
        return true;
    }
    
    // Cancel an existing order
    bool cancelOrder(const OrderId& orderId) {
        auto it = orderLocations_.find(orderId);
        if (it == orderLocations_.end()) {
            return false;
        }
        
        Side side = it->second.first;
        Price price = it->second.second;
        
        bool removed = false;
        if (side == Side::BUY) {
            auto levelIt = buyLevels_.find(price);
            if (levelIt != buyLevels_.end()) {
                removed = levelIt->second.removeOrder(orderId);
                if (levelIt->second.isEmpty()) {
                    buyLevels_.erase(levelIt);
                }
            }
        } else {
            auto levelIt = sellLevels_.find(price);
            if (levelIt != sellLevels_.end()) {
                removed = levelIt->second.removeOrder(orderId);
                if (levelIt->second.isEmpty()) {
                    sellLevels_.erase(levelIt);
                }
            }
        }
        
        if (removed) {
            orderLocations_.erase(orderId);
            return true;
        }
        
        return false;
    }
    
    // Modify the quantity of an existing order
    bool modifyOrder(const OrderId& orderId, Quantity newQuantity) {
        auto it = orderLocations_.find(orderId);
        if (it == orderLocations_.end()) {
            return false;
        }
        
        Side side = it->second.first;
        Price price = it->second.second;
        
        bool modified = false;
        if (side == Side::BUY) {
            auto levelIt = buyLevels_.find(price);
            if (levelIt != buyLevels_.end()) {
                modified = levelIt->second.modifyOrderQuantity(orderId, newQuantity);
            }
        } else {
            auto levelIt = sellLevels_.find(price);
            if (levelIt != sellLevels_.end()) {
                modified = levelIt->second.modifyOrderQuantity(orderId, newQuantity);
            }
        }
        
        // Check for matches after modification
        if (modified) {
            matchOrders();
            return true;
        }
        
        return false;
    }
    
    // Match orders in the book
    void matchOrders() {
        while (!buyLevels_.empty() && !sellLevels_.empty()) {
            auto bestBuyIt = buyLevels_.begin();
            auto bestSellIt = sellLevels_.begin();
            
            if (bestBuyIt->first >= bestSellIt->first) {
                // We have a match!
                executeMatch(bestBuyIt->second, bestSellIt->second);
                
                // Clean up empty levels
                if (bestBuyIt->second.isEmpty()) {
                    buyLevels_.erase(bestBuyIt);
                }
                
                if (bestSellIt->second.isEmpty()) {
                    sellLevels_.erase(bestSellIt);
                }
            } else {
                // No matches possible
                break;
            }
        }
    }
    
    // Execute a match between two price levels
    void executeMatch(PriceLevel& buyLevel, PriceLevel& sellLevel) {
        const Order* buyOrder = buyLevel.getFirstOrder();
        const Order* sellOrder = sellLevel.getFirstOrder();
        
        if (!buyOrder || !sellOrder) return;
        
        // Determine match quantity
        Quantity matchQty = std::min(buyOrder->quantity, sellOrder->quantity);
        
        // Get execution price (typically the resting order's price)
        Price execPrice = buyOrder->timestamp < sellOrder->timestamp ? 
                          buyOrder->price : sellOrder->price;
        
        // In a real system, we would publish the trade here
        std::cout << "TRADE: " << symbol_ << " " << matchQty << " @ " 
                  << execPrice << " (" << buyOrder->id << " x " 
                  << sellOrder->id << ")" << std::endl;
        
        // Process the orders
        Order buyOrderCopy = buyLevel.popFirstOrder();
        Order sellOrderCopy = sellLevel.popFirstOrder();
        
        // Remove from location map
        orderLocations_.erase(buyOrderCopy.id);
        orderLocations_.erase(sellOrderCopy.id);
        
        // If there's remaining quantity, add back to the book
        buyOrderCopy.quantity -= matchQty;
        if (buyOrderCopy.quantity > 0) {
            orderLocations_[buyOrderCopy.id] = {Side::BUY, buyOrderCopy.price};
            buyLevel.addOrder(buyOrderCopy);
        }
        
        sellOrderCopy.quantity -= matchQty;
        if (sellOrderCopy.quantity > 0) {
            orderLocations_[sellOrderCopy.id] = {Side::SELL, sellOrderCopy.price};
            sellLevel.addOrder(sellOrderCopy);
        }
    }
    
    // Get best bid (highest buy price)
    std::pair<Price, Quantity> getBestBid() const {
        if (buyLevels_.empty()) return {0, 0};
        const auto& bestLevel = buyLevels_.begin()->second;
        return {bestLevel.getPrice(), bestLevel.getTotalQuantity()};
    }
    
    // Get best ask (lowest sell price)
    std::pair<Price, Quantity> getBestAsk() const {
        if (sellLevels_.empty()) return {0, 0};
        const auto& bestLevel = sellLevels_.begin()->second;
        return {bestLevel.getPrice(), bestLevel.getTotalQuantity()};
    }
    
    // Get current spread
    Price getSpread() const {
        auto bid = getBestBid();
        auto ask = getBestAsk();
        
        if (bid.first == 0 || ask.first == 0) return -1; // No spread available
        return ask.first - bid.first;
    }
    
    // Print the order book (for debugging)
    void printOrderBook(int depth = 5) const {
        std::cout << "\nOrder Book for " << symbol_ << ":\n";
        
        std::cout << "SELL SIDE:\n";
        int sellCount = 0;
        for (auto it = sellLevels_.begin(); it != sellLevels_.end() && sellCount < depth; ++it, ++sellCount) {
            std::cout << it->first << ": " << it->second.getTotalQuantity() << "\n";
        }
        
        std::cout << "-------------\n";
        
        std::cout << "BUY SIDE:\n";
        int buyCount = 0;
        for (auto it = buyLevels_.begin(); it != buyLevels_.end() && buyCount < depth; ++it, ++buyCount) {
            std::cout << it->first << ": " << it->second.getTotalQuantity() << "\n";
        }
        
        std::cout << std::endl;
    }
};
```

### Key Features:

1. **Price-Time Priority**: Orders are prioritized first by price, then by time of arrival
2. **Efficient Matching**: O(1) access to best bid/ask for quick matching
3. **Order Tracking**: Fast lookup of orders by ID for modifications and cancellations
4. **Memory Efficiency**: Only maintains price levels with active orders
5. **Thread Safety**: Can be made thread-safe with appropriate locks (not shown)

## 2. Market Data Buffer

A circular buffer provides efficient storage and retrieval of time-series market data:

```cpp
#include <vector>
#include <mutex>
#include <atomic>
#include <optional>
#include <iostream>

struct MarketDataUpdate {
    uint64_t timestamp;      // Timestamp in nanoseconds
    uint64_t sequenceNumber; // Sequence number from exchange
    std::string symbol;      // Instrument symbol
    double bidPrice;         // Best bid price
    double askPrice;         // Best ask price
    double bidSize;          // Best bid size
    double askSize;          // Best ask size
    double lastPrice;        // Last trade price
    double lastSize;         // Last trade size
};

class MarketDataBuffer {
private:
    std::vector<MarketDataUpdate> buffer_;
    size_t capacity_;
    std::atomic<size_t> head_; // Write position
    std::atomic<size_t> tail_; // Oldest available position
    std::atomic<uint64_t> count_; // Number of items
    std::mutex mutex_;
    
    // Track latest sequence number per symbol for gap detection
    std::unordered_map<std::string, uint64_t> latestSequence_;
    
public:
    MarketDataBuffer(size_t capacity) 
        : capacity_(capacity), 
          buffer_(capacity), 
          head_(0), 
          tail_(0),
          count_(0) {}
    
    // Add a new market data update
    bool addUpdate(const MarketDataUpdate& update) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Check for sequence gaps
        auto it = latestSequence_.find(update.symbol);
        if (it != latestSequence_.end()) {
            if (update.sequenceNumber != it->second + 1) {
                std::cerr << "Sequence gap detected for " << update.symbol 
                          << ": expected " << (it->second + 1) 
                          << ", got " << update.sequenceNumber << std::endl;
                // In a real system, we would trigger recovery here
            }
        }
        
        // Update latest sequence number
        latestSequence_[update.symbol] = update.sequenceNumber;
        
        // Add to buffer
        buffer_[head_] = update;
        head_ = (head_ + 1) % capacity_;
        
        // If buffer is full, advance tail
        if (count_ == capacity_) {
            tail_ = (tail_ + 1) % capacity_;
        } else {
            count_++;
        }
        
        return true;
    }
    
    // Get the latest update for a symbol
    std::optional<MarketDataUpdate> getLatest(const std::string& symbol) const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (count_ == 0) return std::nullopt;
        
        // Search from newest to oldest
        size_t current = (head_ + capacity_ - 1) % capacity_;
        for (size_t i = 0; i < count_; i++) {
            if (buffer_[current].symbol == symbol) {
                return buffer_[current];
            }
            current = (current + capacity_ - 1) % capacity_;
        }
        
        return std::nullopt;
    }
    
    // Get updates within a time range
    std::vector<MarketDataUpdate> getRange(
        const std::string& symbol, 
        uint64_t startTime, 
        uint64_t endTime) const {
        
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<MarketDataUpdate> result;
        
        if (count_ == 0) return result;
        
        size_t current = tail_;
        for (size_t i = 0; i < count_; i++) {
            const auto& update = buffer_[current];
            if (update.symbol == symbol && 
                update.timestamp >= startTime && 
                update.timestamp <= endTime) {
                result.push_back(update);
            }
            current = (current + 1) % capacity_;
        }
        
        return result;
    }
    
    // Get last N updates for a symbol
    std::vector<MarketDataUpdate> getLastN(
        const std::string& symbol, 
        size_t n) const {
        
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<MarketDataUpdate> result;
        
        if (count_ == 0) return result;
        
        // Search from newest to oldest
        size_t current = (head_ + capacity_ - 1) % capacity_;
        for (size_t i = 0; i < count_ && result.size() < n; i++) {
            if (buffer_[current].symbol == symbol) {
                result.push_back(buffer_[current]);
            }
            current = (current + capacity_ - 1) % capacity_;
        }
        
        return result;
    }
    
    // Get VWAP (Volume-Weighted Average Price) for a symbol over a time range
    double getVWAP(
        const std::string& symbol, 
        uint64_t startTime, 
        uint64_t endTime) const {
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        double totalValue = 0.0;
        double totalVolume = 0.0;
        
        if (count_ == 0) return 0.0;
        
        size_t current = tail_;
        for (size_t i = 0; i < count_; i++) {
            const auto& update = buffer_[current];
            if (update.symbol == symbol && 
                update.timestamp >= startTime && 
                update.timestamp <= endTime && 
                update.lastSize > 0) {
                
                totalValue += update.lastPrice * update.lastSize;
                totalVolume += update.lastSize;
            }
            current = (current + 1) % capacity_;
        }
        
        if (totalVolume == 0.0) return 0.0;
        return totalValue / totalVolume;
    }
    
    // Get statistics about the buffer
    void getStats(size_t& capacity, size_t& count) const {
        capacity = capacity_;
        count = count_;
    }
};
```

### Key Features:

1. **Efficient Memory Usage**: Fixed-size circular buffer prevents unbounded growth
2. **Low-Latency Access**: O(1) append operations for new data
3. **Flexible Querying**: Supports retrieving by symbol, time range, or count
4. **Sequence Tracking**: Detects and logs sequence gaps in market data
5. **Thread Safety**: Uses mutexes to ensure thread-safe operations

## 3. Strategy Configuration Store

This implementation provides a flexible, versioned configuration store for trading strategies:

```cpp
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <variant>
#include <mutex>
#include <chrono>
#include <functional>

// Parameter value can be different types
using ParameterValue = std::variant<
    bool,
    int64_t,
    double,
    std::string,
    std::vector<int64_t>,
    std::vector<double>,
    std::vector<std::string>
>;

struct StrategyParameter {
    std::string name;
    ParameterValue value;
    std::string description;
    bool isRequired;
    uint64_t lastModified; // Timestamp of last modification
};

struct StrategyConfigVersion {
    uint32_t version;
    uint64_t timestamp;
    std::unordered_map<std::string, StrategyParameter> parameters;
    bool isActive;
};

class StrategyConfigStore {
private:
    std::mutex mutex_;
    
    // Map from strategy name to its versions
    std::unordered_map<
        std::string, 
        std::vector<StrategyConfigVersion>
    > strategyVersions_;
    
    // Map from strategy name to observer functions
    std::unordered_map<
        std::string, 
        std::vector<std::function<void(const std::string&, uint32_t)>>
    > changeObservers_;
    
    // Get current timestamp
    uint64_t getCurrentTimestamp() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }
    
public:
    // Add or update a strategy configuration
    uint32_t addOrUpdateStrategy(
        const std::string& strategyName,
        const std::unordered_map<std::string, StrategyParameter>& parameters) {
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        uint32_t newVersion = 1;
        uint64_t timestamp = getCurrentTimestamp();
        
        // Check if strategy exists
        auto& versions = strategyVersions_[strategyName];
        if (!versions.empty()) {
            newVersion = versions.back().version + 1;
            
            // Deactivate current active version
            for (auto& version : versions) {
                if (version.isActive) {
                    version.isActive = false;
                    break;
                }
            }
        }
        
        // Create new version
        StrategyConfigVersion newConfig{
            newVersion,
            timestamp,
            parameters,
            true // New version is active
        };
        
        versions.push_back(std::move(newConfig));
        
        // Notify observers
        auto it = changeObservers_.find(strategyName);
        if (it != changeObservers_.end()) {
            for (const auto& observer : it->second) {
                observer(strategyName, newVersion);
            }
        }
        
        return newVersion;
    }
    
    // Get active configuration for a strategy
    std::optional<StrategyConfigVersion> getActiveConfig(
        const std::string& strategyName) const {
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = strategyVersions_.find(strategyName);
        if (it == strategyVersions_.end()) {
            return std::nullopt;
        }
        
        const auto& versions = it->second;
        for (const auto& version : versions) {
            if (version.isActive) {
                return version;
            }
        }
        
        return std::nullopt;
    }
    
    // Get a specific version of a strategy configuration
    std::optional<StrategyConfigVersion> getConfigVersion(
        const std::string& strategyName, 
        uint32_t version) const {
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = strategyVersions_.find(strategyName);
        if (it == strategyVersions_.end()) {
            return std::nullopt;
        }
        
        const auto& versions = it->second;
        for (const auto& ver : versions) {
            if (ver.version == version) {
                return ver;
            }
        }
        
        return std::nullopt;
    }
    
    // Activate a specific version of a strategy
    bool activateVersion(const std::string& strategyName, uint32_t version) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = strategyVersions_.find(strategyName);
        if (it == strategyVersions_.end()) {
            return false;
        }
        
        auto& versions = it->second;
        bool found = false;
        
        // First pass: deactivate current active version
        for (auto& ver : versions) {
            if (ver.version == version) {
                found = true;
            }
            
            if (ver.isActive) {
                ver.isActive = false;
            }
        }
        
        if (!found) return false;
        
        // Second pass: activate requested version
        for (auto& ver : versions) {
            if (ver.version == version) {
                ver.isActive = true;
                
                // Notify observers
                auto obsIt = changeObservers_.find(strategyName);
                if (obsIt != changeObservers_.end()) {
                    for (const auto& observer : obsIt->second) {
                        observer(strategyName, version);
                    }
                }
                
                return true;
            }
        }
        
        return false; // Should never reach here
    }
    
    // Register for configuration changes
    void registerChangeObserver(
        const std::string& strategyName,
        std::function<void(const std::string&, uint32_t)> callback) {
        
        std::lock_guard<std::mutex> lock(mutex_);
        changeObservers_[strategyName].push_back(std::move(callback));
    }
    
    // Get all available strategies
    std::vector<std::string> getAllStrategyNames() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::vector<std::string> result;
        result.reserve(strategyVersions_.size());
        
        for (const auto& [name, _] : strategyVersions_) {
            result.push_back(name);
        }
        
        return result;
    }
    
    // Get version history for a strategy
    std::vector<uint32_t> getVersionHistory(const std::string& strategyName) const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::vector<uint32_t> result;
        
        auto it = strategyVersions_.find(strategyName);
        if (it == strategyVersions_.end()) {
            return result;
        }
        
        const auto& versions = it->second;
        result.reserve(versions.size());
        
        for (const auto& version : versions) {
            result.push_back(version.version);
        }
        
        return result;
    }
};
```

### Key Features:

1. **Versioning**: Maintains history of all configuration changes
2. **Type Safety**: Uses std::variant for type-safe parameter values
3. **Observer Pattern**: Notifies subscribers about configuration changes
4. **Concurrency**: Thread-safe operations with mutex protection
5. **Flexibility**: Supports different parameter types and multiple strategies

## Grading Considerations

When evaluating student implementations for Part 2, consider:

1. **Performance**: Data structures should be optimized for the expected operations
2. **Correctness**: Key functionality must work as specified
3. **Memory Management**: Efficient use of memory, avoiding leaks or excessive copying
4. **Edge Cases**: Handling of empty states, boundary conditions, concurrency
5. **API Design**: Clean, intuitive interfaces with appropriate abstraction

## Common Issues and Feedback Points

- **Order Book**:
  - Price-time priority is often implemented incorrectly
  - Students may forget to handle empty price levels after order removal
  - Matching engine efficiency is critical - O(1) access to best bid/ask

- **Market Data Buffer**:
  - Students often struggle with the circular buffer implementation
  - Thread safety and race conditions can be problematic
  - Sequence number tracking is frequently overlooked

- **Strategy Configuration**:
  - Version management complexity is sometimes underestimated
  - Type-safe parameter handling requires careful design
  - Observer notification timing can cause subtle bugs 
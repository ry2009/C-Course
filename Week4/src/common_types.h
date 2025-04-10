#pragma once

#include <string>
#include <cstdint>
#include <vector>

/**
 * @brief Represents a market data update from an exchange or data provider
 */
struct MarketDataUpdate {
    int64_t timestamp;         // Timestamp in nanoseconds
    int64_t sequenceNumber;    // Sequence number from source
    std::string symbol;        // Instrument symbol
    
    // Level 1 market data
    double bidPrice;           // Best bid price
    double askPrice;           // Best ask price
    double bidSize;            // Best bid size
    double askSize;            // Best ask size
    
    // Last trade information
    double lastPrice;          // Last trade price
    double lastSize;           // Last trade size
    char lastExchange;         // Exchange where last trade occurred
    
    // Additional metrics
    double volume;             // Total volume for the day
    double vwap;               // Volume-weighted average price
    double openPrice;          // Opening price
    double highPrice;          // Highest price of the day
    double lowPrice;           // Lowest price of the day
    double closePrice;         // Previous closing price
};

/**
 * @brief Represents the side of an order (buy or sell)
 */
enum class OrderSide {
    BUY,
    SELL
};

/**
 * @brief Represents the type of an order
 */
enum class OrderType {
    MARKET,
    LIMIT,
    STOP,
    STOP_LIMIT,
    IOC      // Immediate or Cancel
};

/**
 * @brief Represents the status of an order
 */
enum class OrderStatus {
    NEW,
    PARTIALLY_FILLED,
    FILLED,
    CANCELED,
    REJECTED,
    PENDING_NEW,
    PENDING_CANCEL
};

/**
 * @brief Represents a trading order
 */
struct Order {
    std::string orderId;       // Unique order identifier
    std::string symbol;        // Instrument symbol
    OrderSide side;            // Buy or sell
    OrderType type;            // Type of order
    double quantity;           // Quantity to trade
    double price;              // Limit price (if applicable)
    double stopPrice;          // Stop price (if applicable)
    int64_t timestamp;         // Order creation time
    OrderStatus status;        // Current status
    std::string clientId;      // Client identifier
    std::string strategyId;    // Strategy that generated this order
};

/**
 * @brief Represents a trade execution
 */
struct Execution {
    std::string executionId;   // Unique execution identifier
    std::string orderId;       // Order identifier
    std::string symbol;        // Instrument symbol
    OrderSide side;            // Buy or sell
    double quantity;           // Quantity executed
    double price;              // Execution price
    int64_t timestamp;         // Execution time
    std::string venue;         // Execution venue
    double fee;                // Execution fee
};

/**
 * @brief Represents a trading position
 */
struct Position {
    std::string symbol;        // Instrument symbol
    double quantity;           // Current position quantity
    double averagePrice;       // Average entry price
    double marketPrice;        // Current market price
    double marketValue;        // Current market value
    double unrealizedPnl;      // Unrealized profit/loss
    double realizedPnl;        // Realized profit/loss
    int64_t lastUpdateTime;    // Last position update time
};

/**
 * @brief Represents a portfolio of positions
 */
struct Portfolio {
    std::string portfolioId;   // Portfolio identifier
    std::vector<Position> positions;  // Positions in the portfolio
    double totalValue;         // Total portfolio value
    double totalUnrealizedPnl; // Total unrealized P&L
    double totalRealizedPnl;   // Total realized P&L
};

/**
 * @brief Security type enumeration
 */
enum class SecurityType {
    EQUITY,
    FUTURE,
    OPTION,
    FOREX,
    CRYPTO,
    BOND,
    OTHER
};

/**
 * @brief Represents security reference data
 */
struct SecurityInfo {
    std::string symbol;        // Symbol
    std::string name;          // Full name
    SecurityType type;         // Security type
    std::string exchange;      // Primary exchange
    std::string currency;      // Currency
    double tickSize;           // Minimum price increment
    double multiplier;         // Contract multiplier
    double marginRequirement;  // Margin requirement percentage
}; 
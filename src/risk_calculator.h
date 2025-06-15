#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include "order_book_processor.h"
#include <map>

// Forward declarations
struct MarketDataUpdate;
struct Position;

/**
 * @brief Class for risk calculation and management
 * 
 * Students should implement this class to fulfill the requirements of Part 3:
 * - Calculate Value at Risk (VaR) using historical simulation
 * - Monitor position limits across multiple securities
 * - Implement circuit breakers for extreme market conditions
 */
class RiskCalculator {
public:
    RiskCalculator();
    virtual ~RiskCalculator() = default;
    
    /**
     * @brief Initialize the risk calculator with parameters
     * @param params Configuration parameters for risk calculations
     * @return true if initialization successful, false otherwise
     */
    virtual bool initialize(const std::unordered_map<std::string, double>& params) = 0;
    
    /**
     * @brief Calculate Value at Risk (VaR) using historical simulation
     * 
     * Implement a historical simulation approach to calculate VaR:
     * 1. Use historical market data to create a distribution of potential returns
     * 2. Apply these returns to current positions to simulate potential P&L
     * 3. Determine the loss threshold at the specified confidence level
     * 
     * @param positions Current positions across all securities
     * @param historicalData Historical market data for VaR calculation
     * @param confidenceLevel Confidence level (e.g., 0.95 for 95% confidence)
     * @param horizonDays Time horizon in days
     * @return Value at Risk amount
     */
    virtual double calculateHistoricalVaR(
        const std::unordered_map<std::string, Position>& positions,
        const std::unordered_map<std::string, std::vector<MarketDataUpdate>>& historicalData,
        double confidenceLevel,
        int horizonDays) = 0;
    
    /**
     * @brief Check if positions are within defined limits
     * 
     * Verify that current positions do not exceed predefined limits:
     * - Absolute position size limits per security
     * - Notional value limits
     * - Sector/group exposure limits
     * 
     * @param positions Current positions across all securities
     * @param limits Position limits configuration
     * @return true if all positions are within limits, false otherwise
     */
    virtual bool checkPositionLimits(
        const std::unordered_map<std::string, Position>& positions,
        const std::unordered_map<std::string, double>& limits) = 0;
    
    /**
     * @brief Implement circuit breakers for extreme market conditions
     * 
     * Detect and respond to extreme market conditions by:
     * 1. Identifying rapid price movements
     * 2. Measuring abnormal volatility
     * 3. Detecting liquidity problems
     * 4. Triggering appropriate risk-reduction actions
     * 
     * @param marketData Recent market data updates
     * @param positions Current positions
     * @param thresholds Circuit breaker threshold parameters
     * @return Circuit breaker activation status (0 = normal, 1 = warning, 2 = stop trading)
     */
    virtual int checkCircuitBreakers(
        const std::vector<MarketDataUpdate>& marketData,
        const std::unordered_map<std::string, Position>& positions,
        const std::unordered_map<std::string, double>& thresholds) = 0;
    
    /**
     * @brief Get a detailed risk report for current positions
     * @param positions Current positions
     * @return Risk metrics for all positions
     */
    virtual std::unordered_map<std::string, double> getRiskReport(
        const std::unordered_map<std::string, Position>& positions) = 0;
    
    // Order validation
    bool validateOrder(const Order& order, const Position& position);
    
    // Risk metrics
    double calculateRisk(const std::string& symbol);
    double calculatePositionRisk(const Position& position);
    
    // Circuit breakers
    bool checkCircuitBreaker(const MarketDataUpdate& update);
    void updateCircuitBreakerThresholds(const std::string& symbol, double threshold);
    
    // Position management
    void updatePosition(const Position& position);
    Position getPosition(const std::string& symbol) const;
    
protected:
    // Risk parameters
    double varConfidenceLevel_ = 0.95;
    int varHorizon_ = 1;
    double maxPositionLimit_ = 1000000.0;
    double maxDrawdownLimit_ = 0.05;
    
    // Circuit breaker parameters
    double volatilityThreshold_ = 3.0;  // standard deviations
    double priceChangeThreshold_ = 0.05; // 5% price change
    double volumeThreshold_ = 5.0;      // 5x normal volume
    
    std::map<std::string, Position> positions_;
    std::map<std::string, double> circuitBreakerThresholds_;
    
    // Risk parameters
    double maxPositionRisk_ = 0.1;  // 10% of capital
    double maxDailyLoss_ = 0.05;    // 5% of capital
    double volatilityThreshold_ = 0.02; // 2% price move
};

/**
 * @brief Factory function to create a risk calculator
 * @param calculatorType Type of risk calculator to create
 * @return Shared pointer to the created risk calculator
 */
std::shared_ptr<RiskCalculator> createRiskCalculator(const std::string& calculatorType); 
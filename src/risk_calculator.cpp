#include "risk_calculator.h"
#include "common_types.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <iostream>
#include <random>

RiskCalculator::RiskCalculator() {}

// Implementation of a concrete risk calculator
class HistoricalVaRCalculator : public RiskCalculator {
public:
    HistoricalVaRCalculator() : RiskCalculator() {}
    
    bool initialize(const std::unordered_map<std::string, double>& params) override {
        try {
            // Parse and validate parameters
            auto it = params.find("varConfidenceLevel");
            if (it != params.end()) {
                varConfidenceLevel_ = it->second;
                if (varConfidenceLevel_ <= 0.0 || varConfidenceLevel_ >= 1.0) {
                    std::cerr << "Invalid confidence level: " << varConfidenceLevel_ << std::endl;
                    return false;
                }
            }
            
            it = params.find("varHorizon");
            if (it != params.end()) {
                varHorizon_ = static_cast<int>(it->second);
                if (varHorizon_ <= 0) {
                    std::cerr << "Invalid horizon: " << varHorizon_ << std::endl;
                    return false;
                }
            }
            
            it = params.find("maxPositionLimit");
            if (it != params.end()) {
                maxPositionLimit_ = it->second;
                if (maxPositionLimit_ <= 0.0) {
                    std::cerr << "Invalid position limit: " << maxPositionLimit_ << std::endl;
                    return false;
                }
            }
            
            it = params.find("volatilityThreshold");
            if (it != params.end()) {
                volatilityThreshold_ = it->second;
            }
            
            it = params.find("priceChangeThreshold");
            if (it != params.end()) {
                priceChangeThreshold_ = it->second;
            }
            
            it = params.find("volumeThreshold");
            if (it != params.end()) {
                volumeThreshold_ = it->second;
            }
            
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error initializing risk calculator: " << e.what() << std::endl;
            return false;
        }
    }
    
    double calculateHistoricalVaR(
        const std::unordered_map<std::string, Position>& positions,
        const std::unordered_map<std::string, std::vector<MarketDataUpdate>>& historicalData,
        double confidenceLevel,
        int horizonDays) override {
        
        // Ensure we have positions and historical data
        if (positions.empty() || historicalData.empty()) {
            return 0.0;
        }
        
        // Calculate current portfolio value
        double currentPortfolioValue = 0.0;
        for (const auto& [symbol, position] : positions) {
            currentPortfolioValue += position.marketValue;
        }
        
        if (currentPortfolioValue <= 0.0) {
            return 0.0; // No exposure
        }
        
        // Calculate historical returns for each security
        std::unordered_map<std::string, std::vector<double>> historicalReturns;
        
        for (const auto& [symbol, updates] : historicalData) {
            if (updates.size() < 2) continue;
            
            // Calculate daily returns from historical data
            std::vector<double> returns;
            returns.reserve(updates.size() - 1);
            
            // Sort by timestamp
            auto sortedUpdates = updates;
            std::sort(sortedUpdates.begin(), sortedUpdates.end(),
                     [](const MarketDataUpdate& a, const MarketDataUpdate& b) {
                         return a.timestamp < b.timestamp;
                     });
            
            // Calculate returns
            for (size_t i = 1; i < sortedUpdates.size(); ++i) {
                double prevPrice = sortedUpdates[i-1].lastPrice;
                double currentPrice = sortedUpdates[i].lastPrice;
                
                if (prevPrice > 0.0) {
                    double dailyReturn = (currentPrice - prevPrice) / prevPrice;
                    returns.push_back(dailyReturn);
                }
            }
            
            historicalReturns[symbol] = returns;
        }
        
        // Generate portfolio returns scenarios using historical simulation
        std::vector<double> portfolioReturns;
        int maxScenarios = 0;
        
        // Find the maximum number of scenarios we can generate
        for (const auto& [symbol, returns] : historicalReturns) {
            if (positions.find(symbol) != positions.end()) {
                maxScenarios = std::max(maxScenarios, static_cast<int>(returns.size()));
            }
        }
        
        // Limit to a reasonable number for performance
        const int MAX_SCENARIOS = 1000;
        maxScenarios = std::min(maxScenarios, MAX_SCENARIOS);
        
        if (maxScenarios <= 0) {
            return 0.0; // Not enough data
        }
        
        portfolioReturns.reserve(maxScenarios);
        
        // Generate scenarios
        for (int scenario = 0; scenario < maxScenarios; ++scenario) {
            double scenarioReturn = 0.0;
            
            for (const auto& [symbol, position] : positions) {
                // Skip if no historical data
                if (historicalReturns.find(symbol) == historicalReturns.end() ||
                    historicalReturns[symbol].empty()) {
                    continue;
                }
                
                // Get the return for this scenario (cycling if needed)
                const auto& returns = historicalReturns[symbol];
                double returnRate = returns[scenario % returns.size()];
                
                // Scale return by horizon (simple scaling for demonstration)
                double scaledReturn = returnRate * std::sqrt(static_cast<double>(horizonDays));
                
                // Weight by position value relative to portfolio
                double weight = position.marketValue / currentPortfolioValue;
                scenarioReturn += scaledReturn * weight;
            }
            
            portfolioReturns.push_back(scenarioReturn);
        }
        
        // Sort portfolio returns to find the VaR percentile
        std::sort(portfolioReturns.begin(), portfolioReturns.end());
        
        // Find the VaR at the specified confidence level
        int varIndex = static_cast<int>(portfolioReturns.size() * (1.0 - confidenceLevel));
        varIndex = std::max(0, std::min(varIndex, static_cast<int>(portfolioReturns.size()) - 1));
        
        // VaR is the loss, so we negate the return (if it's negative)
        double varReturn = portfolioReturns[varIndex];
        double varAmount = -varReturn * currentPortfolioValue; // Convert to dollar amount
        
        // Only return positive VaR (losses)
        return std::max(0.0, varAmount);
    }
    
    bool checkPositionLimits(
        const std::unordered_map<std::string, Position>& positions,
        const std::unordered_map<std::string, double>& limits) override {
        
        if (positions.empty()) {
            return true; // No positions, no violations
        }
        
        bool withinLimits = true;
        
        // Track total exposure by various dimensions
        double totalGrossExposure = 0.0;
        double totalLongExposure = 0.0;
        double totalShortExposure = 0.0;
        std::unordered_map<std::string, double> sectorExposure;
        
        // Check individual position limits
        for (const auto& [symbol, position] : positions) {
            // Absolute position value
            double positionValue = std::abs(position.marketValue);
            totalGrossExposure += positionValue;
            
            // Long/short exposure
            if (position.quantity > 0) {
                totalLongExposure += positionValue;
            } else if (position.quantity < 0) {
                totalShortExposure += positionValue;
            }
            
            // Check symbol-specific limit
            auto symbolLimitIt = limits.find(symbol);
            if (symbolLimitIt != limits.end() && positionValue > symbolLimitIt->second) {
                std::cout << "Position limit violation for " << symbol 
                          << ": " << positionValue << " > " << symbolLimitIt->second << std::endl;
                withinLimits = false;
            }
            
            // Check sector limits (simplified - in a real system we'd have sector mappings)
            std::string sector = symbol.substr(0, 2); // Use first 2 chars as proxy for sector
            sectorExposure[sector] += positionValue;
        }
        
        // Check total exposure limit
        auto totalLimitIt = limits.find("TOTAL_EXPOSURE");
        if (totalLimitIt != limits.end() && totalGrossExposure > totalLimitIt->second) {
            std::cout << "Total exposure limit violation: " 
                      << totalGrossExposure << " > " << totalLimitIt->second << std::endl;
            withinLimits = false;
        }
        
        // Check long exposure limit
        auto longLimitIt = limits.find("LONG_EXPOSURE");
        if (longLimitIt != limits.end() && totalLongExposure > longLimitIt->second) {
            std::cout << "Long exposure limit violation: " 
                      << totalLongExposure << " > " << longLimitIt->second << std::endl;
            withinLimits = false;
        }
        
        // Check short exposure limit
        auto shortLimitIt = limits.find("SHORT_EXPOSURE");
        if (shortLimitIt != limits.end() && totalShortExposure > shortLimitIt->second) {
            std::cout << "Short exposure limit violation: " 
                      << totalShortExposure << " > " << shortLimitIt->second << std::endl;
            withinLimits = false;
        }
        
        // Check sector limits
        for (const auto& [sector, exposure] : sectorExposure) {
            auto sectorLimitIt = limits.find("SECTOR_" + sector);
            if (sectorLimitIt != limits.end() && exposure > sectorLimitIt->second) {
                std::cout << "Sector " << sector << " exposure limit violation: " 
                          << exposure << " > " << sectorLimitIt->second << std::endl;
                withinLimits = false;
            }
        }
        
        return withinLimits;
    }
    
    int checkCircuitBreakers(
        const std::vector<MarketDataUpdate>& marketData,
        const std::unordered_map<std::string, Position>& positions,
        const std::unordered_map<std::string, double>& thresholds) override {
        
        if (marketData.empty() || positions.empty()) {
            return 0; // No data or positions, no circuit breakers
        }
        
        // Group market data by symbol and sort by timestamp
        std::unordered_map<std::string, std::vector<MarketDataUpdate>> symbolData;
        
        for (const auto& update : marketData) {
            symbolData[update.symbol].push_back(update);
        }
        
        // Sort each symbol's data by timestamp
        for (auto& [symbol, updates] : symbolData) {
            std::sort(updates.begin(), updates.end(),
                     [](const MarketDataUpdate& a, const MarketDataUpdate& b) {
                         return a.timestamp < b.timestamp;
                     });
        }
        
        // Initialize circuit breaker status
        int circuitBreakerStatus = 0; // 0 = normal, 1 = warning, 2 = stop trading
        
        // Check for rapid price movements
        for (const auto& [symbol, position] : positions) {
            // Skip if no market data
            if (symbolData.find(symbol) == symbolData.end() || 
                symbolData[symbol].size() < 2) {
                continue;
            }
            
            const auto& updates = symbolData[symbol];
            
            // Get the most recent and oldest updates
            const auto& latestUpdate = updates.back();
            const auto& oldestUpdate = updates.front();
            
            // Calculate price change
            double priceChange = (latestUpdate.lastPrice - oldestUpdate.lastPrice) / oldestUpdate.lastPrice;
            double absChange = std::abs(priceChange);
            
            // Get custom threshold or use default
            double priceThreshold = priceChangeThreshold_;
            auto customThresholdIt = thresholds.find(symbol + "_PRICE_CHANGE");
            if (customThresholdIt != thresholds.end()) {
                priceThreshold = customThresholdIt->second;
            }
            
            // Check price change threshold
            if (absChange > priceThreshold) {
                std::cout << "Circuit breaker: Rapid price movement detected for " << symbol
                          << ": " << (priceChange * 100.0) << "% change" << std::endl;
                circuitBreakerStatus = std::max(circuitBreakerStatus, 1);
                
                // If position is large and price movement is against us, escalate
                if ((position.quantity > 0 && priceChange < 0) || 
                    (position.quantity < 0 && priceChange > 0)) {
                    if (std::abs(position.marketValue) > maxPositionLimit_ * 0.5 && 
                        absChange > priceThreshold * 2.0) {
                        std::cout << "Circuit breaker: Stopping trading due to extreme adverse price movement for "
                                  << symbol << std::endl;
                        circuitBreakerStatus = 2;
                    }
                }
            }
            
            // Check for abnormal volatility using standard deviation of returns
            if (updates.size() >= 5) {
                std::vector<double> returns;
                returns.reserve(updates.size() - 1);
                
                for (size_t i = 1; i < updates.size(); ++i) {
                    double ret = (updates[i].lastPrice - updates[i-1].lastPrice) / updates[i-1].lastPrice;
                    returns.push_back(ret);
                }
                
                // Calculate volatility (standard deviation of returns)
                double mean = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
                double variance = 0.0;
                for (double r : returns) {
                    variance += (r - mean) * (r - mean);
                }
                variance /= returns.size();
                double volatility = std::sqrt(variance);
                
                // Get custom threshold or use default
                double volThreshold = volatilityThreshold_;
                auto customVolThresholdIt = thresholds.find(symbol + "_VOLATILITY");
                if (customVolThresholdIt != thresholds.end()) {
                    volThreshold = customVolThresholdIt->second;
                }
                
                // Check volatility threshold
                if (volatility > volThreshold) {
                    std::cout << "Circuit breaker: Abnormal volatility detected for " << symbol
                              << ": " << (volatility * 100.0) << "% (threshold: " 
                              << (volThreshold * 100.0) << "%)" << std::endl;
                    circuitBreakerStatus = std::max(circuitBreakerStatus, 1);
                    
                    // If extremely high volatility, escalate
                    if (volatility > volThreshold * 3.0) {
                        std::cout << "Circuit breaker: Stopping trading due to extreme volatility for "
                                  << symbol << std::endl;
                        circuitBreakerStatus = 2;
                    }
                }
            }
            
            // Check for liquidity problems (volume spikes or drops)
            if (updates.size() >= 3) {
                // Calculate average volume
                double totalVolume = 0.0;
                for (const auto& update : updates) {
                    totalVolume += update.volume;
                }
                double avgVolume = totalVolume / updates.size();
                
                // Check latest volume
                double latestVolume = latestUpdate.volume;
                double volumeRatio = latestVolume / (avgVolume > 0 ? avgVolume : 1.0);
                
                // Get custom threshold or use default
                double volumeThreshold = volumeThreshold_;
                auto customVolThresholdIt = thresholds.find(symbol + "_VOLUME");
                if (customVolThresholdIt != thresholds.end()) {
                    volumeThreshold = customVolThresholdIt->second;
                }
                
                // Check for volume spike (could indicate market dislocation)
                if (volumeRatio > volumeThreshold) {
                    std::cout << "Circuit breaker: Volume spike detected for " << symbol
                              << ": " << volumeRatio << "x normal volume" << std::endl;
                    circuitBreakerStatus = std::max(circuitBreakerStatus, 1);
                }
                
                // Check for volume drop (could indicate liquidity problems)
                if (volumeRatio < 1.0 / volumeThreshold) {
                    std::cout << "Circuit breaker: Volume drop detected for " << symbol
                              << ": " << volumeRatio << "x normal volume" << std::endl;
                    circuitBreakerStatus = std::max(circuitBreakerStatus, 1);
                }
            }
        }
        
        return circuitBreakerStatus;
    }
    
    std::unordered_map<std::string, double> getRiskReport(
        const std::unordered_map<std::string, Position>& positions) override {
        
        std::unordered_map<std::string, double> riskMetrics;
        
        if (positions.empty()) {
            return riskMetrics;
        }
        
        // Calculate total portfolio metrics
        double totalValue = 0.0;
        double totalLongValue = 0.0;
        double totalShortValue = 0.0;
        double totalUnrealizedPnl = 0.0;
        double totalRealizedPnl = 0.0;
        
        // Track largest positions
        std::string largestLongSymbol;
        double largestLongValue = 0.0;
        std::string largestShortSymbol;
        double largestShortValue = 0.0;
        
        for (const auto& [symbol, position] : positions) {
            double posValue = position.marketValue;
            totalValue += posValue;
            totalUnrealizedPnl += position.unrealizedPnl;
            totalRealizedPnl += position.realizedPnl;
            
            if (position.quantity > 0) {
                totalLongValue += posValue;
                if (posValue > largestLongValue) {
                    largestLongValue = posValue;
                    largestLongSymbol = symbol;
                }
            } else if (position.quantity < 0) {
                totalShortValue += std::abs(posValue);
                if (std::abs(posValue) > largestShortValue) {
                    largestShortValue = std::abs(posValue);
                    largestShortSymbol = symbol;
                }
            }
        }
        
        // Add metrics to the report
        riskMetrics["TOTAL_VALUE"] = totalValue;
        riskMetrics["TOTAL_LONG_VALUE"] = totalLongValue;
        riskMetrics["TOTAL_SHORT_VALUE"] = totalShortValue;
        riskMetrics["TOTAL_UNREALIZED_PNL"] = totalUnrealizedPnl;
        riskMetrics["TOTAL_REALIZED_PNL"] = totalRealizedPnl;
        riskMetrics["GROSS_EXPOSURE"] = totalLongValue + totalShortValue;
        riskMetrics["NET_EXPOSURE"] = totalLongValue - totalShortValue;
        
        // Calculate exposure ratios
        if (totalValue != 0.0) {
            riskMetrics["LONG_RATIO"] = totalLongValue / std::abs(totalValue);
            riskMetrics["SHORT_RATIO"] = totalShortValue / std::abs(totalValue);
        }
        
        // Add largest position metrics
        if (!largestLongSymbol.empty()) {
            riskMetrics["LARGEST_LONG_VALUE"] = largestLongValue;
            riskMetrics["LARGEST_LONG_PCT"] = largestLongValue / (totalValue != 0.0 ? std::abs(totalValue) : 1.0);
        }
        
        if (!largestShortSymbol.empty()) {
            riskMetrics["LARGEST_SHORT_VALUE"] = largestShortValue;
            riskMetrics["LARGEST_SHORT_PCT"] = largestShortValue / (totalValue != 0.0 ? std::abs(totalValue) : 1.0);
        }
        
        // Calculate concentration metrics
        double herfindahlIndex = 0.0; // Measure of concentration
        for (const auto& [symbol, position] : positions) {
            double weightSquared = std::pow(std::abs(position.marketValue) / 
                                         (totalValue != 0.0 ? std::abs(totalValue) : 1.0), 2);
            herfindahlIndex += weightSquared;
        }
        riskMetrics["CONCENTRATION_INDEX"] = herfindahlIndex;
        
        return riskMetrics;
    }
};

// Factory implementation
std::shared_ptr<RiskCalculator> createRiskCalculator(const std::string& calculatorType) {
    if (calculatorType == "HistoricalVaR") {
        return std::make_shared<HistoricalVaRCalculator>();
    }
    // Add more calculator types here
    
    return nullptr;
} 
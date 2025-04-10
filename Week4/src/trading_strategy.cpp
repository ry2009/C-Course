#include "trading_strategy.h"
#include "common_types.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <random>
#include <unordered_set>
#include <queue>

TradingStrategy::TradingStrategy(const std::string& name) : name_(name) {}

// Implementation of a concrete trading strategy
class MeanReversionStrategy : public TradingStrategy {
public:
    MeanReversionStrategy(const std::string& name) : TradingStrategy(name) {}
    
    bool initialize(const std::unordered_map<std::string, std::string>& params) override {
        try {
            // Parse and validate parameters
            auto it = params.find("lookbackPeriod");
            if (it != params.end()) {
                lookbackPeriod_ = std::stoi(it->second);
            }
            
            it = params.find("deviationThreshold");
            if (it != params.end()) {
                deviationThreshold_ = std::stod(it->second);
            }
            
            it = params.find("positionLimit");
            if (it != params.end()) {
                positionLimit_ = std::stod(it->second);
            }
            
            it = params.find("riskLimit");
            if (it != params.end()) {
                riskLimit_ = std::stod(it->second);
            }
            
            it = params.find("symbols");
            if (it != params.end()) {
                std::string symbolsStr = it->second;
                size_t pos = 0;
                std::string token;
                while ((pos = symbolsStr.find(',')) != std::string::npos) {
                    token = symbolsStr.substr(0, pos);
                    watchlist_.insert(token);
                    symbolsStr.erase(0, pos + 1);
                }
                if (!symbolsStr.empty()) {
                    watchlist_.insert(symbolsStr);
                }
            }
            
            if (lookbackPeriod_ <= 0 || deviationThreshold_ <= 0 || watchlist_.empty()) {
                std::cerr << "Invalid strategy parameters" << std::endl;
                return false;
            }
            
            // Initialize historical data storage
            for (const auto& symbol : watchlist_) {
                priceHistory_[symbol] = std::vector<double>();
                priceHistory_[symbol].reserve(lookbackPeriod_ * 2); // Reserve extra space
                
                // Initialize market relationships graph
                for (const auto& otherSymbol : watchlist_) {
                    if (symbol != otherSymbol) {
                        correlationMatrix_[symbol][otherSymbol] = 0.0;
                    }
                }
            }
            
            isActive_ = true;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error initializing strategy: " << e.what() << std::endl;
            return false;
        }
    }
    
    std::vector<Order> onMarketDataUpdate(const MarketDataUpdate& marketData) override {
        if (!isActive_ || watchlist_.find(marketData.symbol) == watchlist_.end()) {
            return {}; // Not active or not in watchlist
        }
        
        // Store the price in history
        priceHistory_[marketData.symbol].push_back(marketData.lastPrice);
        
        // Keep history within lookback period
        if (priceHistory_[marketData.symbol].size() > static_cast<size_t>(lookbackPeriod_ * 2)) {
            priceHistory_[marketData.symbol].erase(priceHistory_[marketData.symbol].begin());
        }
        
        // Need enough data for mean reversion strategy
        if (priceHistory_[marketData.symbol].size() < static_cast<size_t>(lookbackPeriod_)) {
            return {};
        }
        
        // Process market data using sorting and filtering
        auto processedData = processMarketData({marketData});
        if (processedData.empty()) {
            return {};
        }
        
        // Calculate moving average using the lookback period
        double movingAvg = calculateMovingAverage(marketData.symbol, lookbackPeriod_);
        
        // Calculate standard deviation
        double stdDev = calculateStandardDeviation(marketData.symbol, lookbackPeriod_, movingAvg);
        
        // Calculate z-score (how many standard deviations away from the mean)
        double zScore = (marketData.lastPrice - movingAvg) / (stdDev > 0 ? stdDev : 1.0);
        
        // Generate a trading signal based on mean reversion
        std::vector<Order> orders;
        
        // If price is significantly above MA, consider selling
        if (zScore > deviationThreshold_) {
            // Calculate optimal order size
            double volatility = stdDev / movingAvg; // Normalized volatility
            int orderSize = calculateOptimalOrderSize(marketData.lastPrice, volatility, riskLimit_);
            
            if (orderSize > 0) {
                Order sellOrder;
                sellOrder.symbol = marketData.symbol;
                sellOrder.side = OrderSide::SELL;
                sellOrder.type = OrderType::LIMIT;
                sellOrder.quantity = orderSize;
                sellOrder.price = marketData.bidPrice * 0.999; // Slightly below bid
                sellOrder.timestamp = marketData.timestamp;
                sellOrder.status = OrderStatus::NEW;
                sellOrder.strategyId = getName();
                orders.push_back(sellOrder);
            }
        }
        // If price is significantly below MA, consider buying
        else if (zScore < -deviationThreshold_) {
            // Calculate optimal order size
            double volatility = stdDev / movingAvg; // Normalized volatility
            int orderSize = calculateOptimalOrderSize(marketData.lastPrice, volatility, riskLimit_);
            
            if (orderSize > 0) {
                Order buyOrder;
                buyOrder.symbol = marketData.symbol;
                buyOrder.side = OrderSide::BUY;
                buyOrder.type = OrderType::LIMIT;
                buyOrder.quantity = orderSize;
                buyOrder.price = marketData.askPrice * 1.001; // Slightly above ask
                buyOrder.timestamp = marketData.timestamp;
                buyOrder.status = OrderStatus::NEW;
                buyOrder.strategyId = getName();
                orders.push_back(buyOrder);
            }
        }
        
        // Update market relationships periodically
        if (marketData.timestamp % (1000000000LL * 60) < 1000000) { // Approximately once a minute
            // We would call this in a real system, but it's expensive 
            // so we'll skip for most updates
            // analyzeMarketRelationships(allMarketData);
        }
        
        return orders;
    }
    
    int calculateOptimalOrderSize(double price, double volatility, double maxRisk) override {
        // Dynamic Programming Approach for Order Sizing
        
        // Define the maximum position size
        int maxSize = static_cast<int>(positionLimit_ / price);
        
        // The value we're trying to maximize is expected return with risk constraint
        // We'll use a table to store the optimal order size for each risk level
        std::vector<std::pair<int, double>> dp(maxSize + 1, {0, 0.0});
        
        // Initialize base case
        dp[0] = {0, 0.0}; // Zero size has zero risk
        
        // Fill the DP table
        for (int size = 1; size <= maxSize; ++size) {
            // Calculate risk for this position size (simplified model)
            double risk = price * size * volatility;
            
            // Calculate expected return (simplified mean reversion model)
            // Higher volatility means higher potential return in mean reversion
            double expectedReturn = size * price * volatility * 0.1; 
            
            // Risk-adjusted return
            double riskadjReturn = expectedReturn / (risk > 0 ? risk : 1.0);
            
            // Store size and its risk-adjusted return
            dp[size] = {size, riskadjReturn};
        }
        
        // Find the size with the best risk-adjusted return
        // that doesn't exceed our max risk
        std::sort(dp.begin(), dp.end(), 
            [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
                return a.second > b.second; // Sort by risk-adjusted return (descending)
            });
        
        // Find the first size that doesn't exceed our risk limit
        for (const auto& [size, retRisk] : dp) {
            double totalRisk = price * size * volatility;
            if (totalRisk <= maxRisk) {
                return size;
            }
        }
        
        return 0; // Default to zero if no suitable size found
    }
    
    std::vector<std::vector<double>> analyzeMarketRelationships(
        const std::vector<MarketDataUpdate>& marketData) override {
        
        // Group market data by symbol
        std::unordered_map<std::string, std::vector<double>> symbolPrices;
        for (const auto& update : marketData) {
            if (watchlist_.find(update.symbol) != watchlist_.end()) {
                symbolPrices[update.symbol].push_back(update.lastPrice);
            }
        }
        
        // We need at least 2 symbols with data to calculate correlations
        if (symbolPrices.size() < 2) {
            return {};
        }
        
        // Use graph algorithms to analyze relationships
        calculateCorrelationMatrix(symbolPrices);
        
        // Find closely related securities using graph traversal
        findRelatedSecurities();
        
        // Convert the correlation matrix to a return format
        std::vector<std::vector<double>> result;
        result.resize(watchlist_.size());
        
        int i = 0;
        for (const auto& symbol : watchlist_) {
            result[i].resize(watchlist_.size());
            int j = 0;
            for (const auto& otherSymbol : watchlist_) {
                if (symbol == otherSymbol) {
                    result[i][j] = 1.0; // Self-correlation is always 1
                } else {
                    result[i][j] = correlationMatrix_[symbol][otherSymbol];
                }
                j++;
            }
            i++;
        }
        
        return result;
    }
    
    std::vector<MarketDataUpdate> processMarketData(
        const std::vector<MarketDataUpdate>& marketData) override {
        
        // Copy market data for processing
        std::vector<MarketDataUpdate> filteredData;
        filteredData.reserve(marketData.size());
        
        // Filter: only include symbols in our watchlist
        std::copy_if(marketData.begin(), marketData.end(), 
                    std::back_inserter(filteredData),
                    [this](const MarketDataUpdate& update) {
                        return watchlist_.find(update.symbol) != watchlist_.end();
                    });
        
        // Sort data by timestamp (ascending)
        std::sort(filteredData.begin(), filteredData.end(),
                 [](const MarketDataUpdate& a, const MarketDataUpdate& b) {
                     return a.timestamp < b.timestamp;
                 });
        
        // Filter out stale data (more than 5 seconds old)
        if (!filteredData.empty()) {
            int64_t latestTimestamp = filteredData.back().timestamp;
            filteredData.erase(
                std::remove_if(filteredData.begin(), filteredData.end(),
                              [latestTimestamp](const MarketDataUpdate& update) {
                                  return latestTimestamp - update.timestamp > 5000000000; // 5 seconds in ns
                              }),
                filteredData.end());
        }
        
        return filteredData;
    }
    
private:
    double calculateMovingAverage(const std::string& symbol, int period) {
        const auto& prices = priceHistory_[symbol];
        if (prices.size() < static_cast<size_t>(period)) {
            return 0.0;
        }
        
        auto start = prices.end() - period;
        auto end = prices.end();
        
        double sum = std::accumulate(start, end, 0.0);
        return sum / period;
    }
    
    double calculateStandardDeviation(const std::string& symbol, int period, double mean) {
        const auto& prices = priceHistory_[symbol];
        if (prices.size() < static_cast<size_t>(period)) {
            return 0.0;
        }
        
        auto start = prices.end() - period;
        auto end = prices.end();
        
        double sumSquaredDiff = 0.0;
        for (auto it = start; it != end; ++it) {
            double diff = *it - mean;
            sumSquaredDiff += diff * diff;
        }
        
        return std::sqrt(sumSquaredDiff / period);
    }
    
    void calculateCorrelationMatrix(const std::unordered_map<std::string, std::vector<double>>& symbolPrices) {
        // For each pair of symbols, calculate correlation
        for (const auto& [symbol1, prices1] : symbolPrices) {
            for (const auto& [symbol2, prices2] : symbolPrices) {
                if (symbol1 != symbol2) {
                    // Use the minimum length of both price vectors
                    size_t minLength = std::min(prices1.size(), prices2.size());
                    if (minLength > 1) {
                        // Calculate correlation using Pearson correlation coefficient
                        double correlation = calculateCorrelation(
                            std::vector<double>(prices1.end() - minLength, prices1.end()),
                            std::vector<double>(prices2.end() - minLength, prices2.end())
                        );
                        
                        correlationMatrix_[symbol1][symbol2] = correlation;
                    }
                }
            }
        }
    }
    
    double calculateCorrelation(const std::vector<double>& x, const std::vector<double>& y) {
        double sum_x = std::accumulate(x.begin(), x.end(), 0.0);
        double sum_y = std::accumulate(y.begin(), y.end(), 0.0);
        
        double mean_x = sum_x / x.size();
        double mean_y = sum_y / y.size();
        
        double sum_xy = 0.0, sum_x2 = 0.0, sum_y2 = 0.0;
        
        for (size_t i = 0; i < x.size(); ++i) {
            double x_diff = x[i] - mean_x;
            double y_diff = y[i] - mean_y;
            
            sum_xy += x_diff * y_diff;
            sum_x2 += x_diff * x_diff;
            sum_y2 += y_diff * y_diff;
        }
        
        if (sum_x2 * sum_y2 <= 0.0) {
            return 0.0; // Avoid division by zero
        }
        
        return sum_xy / std::sqrt(sum_x2 * sum_y2);
    }
    
    void findRelatedSecurities() {
        // For each symbol, find its most correlated partners using BFS
        for (const auto& symbol : watchlist_) {
            // Store the correlation and the symbol
            std::priority_queue<std::pair<double, std::string>> correlatedSymbols;
            
            // Add all direct correlations
            for (const auto& [otherSymbol, correlation] : correlationMatrix_[symbol]) {
                correlatedSymbols.push({std::abs(correlation), otherSymbol});
            }
            
            // Print the top 3 most correlated symbols
            std::cout << "Top correlations for " << symbol << ":" << std::endl;
            int count = 0;
            while (!correlatedSymbols.empty() && count < 3) {
                auto [correlation, otherSymbol] = correlatedSymbols.top();
                correlatedSymbols.pop();
                
                if (correlation > 0.0) {
                    std::cout << "  - " << otherSymbol << ": " << correlation << std::endl;
                    count++;
                }
            }
        }
    }
    
    // Strategy parameters
    double deviationThreshold_ = 2.0;  // Z-score threshold for trading signals
    std::unordered_set<std::string> watchlist_; // Symbols to monitor
    
    // Data storage
    std::unordered_map<std::string, std::vector<double>> priceHistory_;
    std::unordered_map<std::string, std::unordered_map<std::string, double>> correlationMatrix_;
};

// Factory implementation
std::shared_ptr<TradingStrategy> createStrategy(const std::string& strategyType, const std::string& name) {
    if (strategyType == "MeanReversion") {
        return std::make_shared<MeanReversionStrategy>(name);
    }
    // Add more strategy types here
    
    return nullptr;
} 
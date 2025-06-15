#include "order_book_processor.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <limits>

// Basic OrderBookEntry implementation
OrderBookEntry::OrderBookEntry(double price, double quantity, OrderSide side, int priority)
    : price(price), quantity(quantity), side(side), priority(priority) {}

// OrderBookProcessor implementation
OrderBookProcessor::OrderBookProcessor() {
    resetMetrics();
}

OrderBookProcessor::~OrderBookProcessor() {
    // Clean up any resources
}

void OrderBookProcessor::resetMetrics() {
    spreadHistory_.clear();
    depthHistory_.clear();
    volumeImbalanceHistory_.clear();
    priceImpactHistory_.clear();
    midPriceHistory_.clear();
    microPriceHistory_.clear();
    marketImpactMetrics_.clear();
    orderBooks_.clear();
    historicalData_.clear();
}

void OrderBookProcessor::processOrder(const Order& order) {
    if (order.orderId.empty()) {
        std::cerr << "Invalid order: missing order ID" << std::endl;
        return;
    }
    
    // Process the order based on its type
    switch (order.type) {
        case OrderType::LIMIT:
            processLimitOrder(order);
            break;
        case OrderType::MARKET:
            processMarketOrder(order);
            break;
        case OrderType::CANCEL:
            processCancelOrder(order);
            break;
        case OrderType::MODIFY:
            processModifyOrder(order);
            break;
        default:
            std::cerr << "Unknown order type for order: " << order.orderId << std::endl;
    }
    
    // Update market metrics after processing the order
    updateMarketMetrics(order.symbol);
}

void OrderBookProcessor::processMarketDataUpdate(const MarketDataUpdate& update) {
    // Store historical data
    if (!update.symbol.empty()) {
        marketDataHistory_[update.symbol].push_back(update);
        
        // Limit history size
        const size_t MAX_HISTORY = 1000;
        if (marketDataHistory_[update.symbol].size() > MAX_HISTORY) {
            marketDataHistory_[update.symbol].erase(marketDataHistory_[update.symbol].begin());
        }
    }
    
    // Update order book if the update contains book information
    if (!update.bids.empty() || !update.asks.empty()) {
        updateOrderBook(update);
    }
    
    // Update metrics
    updateMarketMetrics(update.symbol);
}

void OrderBookProcessor::updateOrderBook(const MarketDataUpdate& update) {
    const std::string& symbol = update.symbol;
    
    // Update bids
    if (!update.bids.empty()) {
        orderBooks_[symbol].bids.clear();
        for (const auto& bid : update.bids) {
            orderBooks_[symbol].bids.emplace_back(
                bid.price, bid.quantity, OrderSide::BUY, bid.priority);
        }
        
        // Sort bids in descending order by price
        std::sort(orderBooks_[symbol].bids.begin(), orderBooks_[symbol].bids.end(),
                 [](const OrderBookEntry& a, const OrderBookEntry& b) {
                     return a.price > b.price;
                 });
    }
    
    // Update asks
    if (!update.asks.empty()) {
        orderBooks_[symbol].asks.clear();
        for (const auto& ask : update.asks) {
            orderBooks_[symbol].asks.emplace_back(
                ask.price, ask.quantity, OrderSide::SELL, ask.priority);
        }
        
        // Sort asks in ascending order by price
        std::sort(orderBooks_[symbol].asks.begin(), orderBooks_[symbol].asks.end(),
                 [](const OrderBookEntry& a, const OrderBookEntry& b) {
                     return a.price < b.price;
                 });
    }
    
    // Update timestamp
    orderBooks_[symbol].lastUpdateTime = update.timestamp;
}

void OrderBookProcessor::processLimitOrder(const Order& order) {
    // Store the order in active orders map
    activeOrders_[order.orderId] = order;
    
    // Apply order to the book
    OrderBook& book = orderBooks_[order.symbol];
    
    if (order.side == OrderSide::BUY) {
        // Add to bids
        book.bids.emplace_back(order.price, order.quantity, OrderSide::BUY, 
                              static_cast<int>(book.bids.size()));
        
        // Sort bids in descending order by price
        std::sort(book.bids.begin(), book.bids.end(),
                 [](const OrderBookEntry& a, const OrderBookEntry& b) {
                     if (std::abs(a.price - b.price) < 1e-6) {
                         return a.priority < b.priority; // Lower priority number = higher priority
                     }
                     return a.price > b.price;
                 });
    } else {
        // Add to asks
        book.asks.emplace_back(order.price, order.quantity, OrderSide::SELL, 
                              static_cast<int>(book.asks.size()));
        
        // Sort asks in ascending order by price
        std::sort(book.asks.begin(), book.asks.end(),
                 [](const OrderBookEntry& a, const OrderBookEntry& b) {
                     if (std::abs(a.price - b.price) < 1e-6) {
                         return a.priority < b.priority;
                     }
                     return a.price < b.price;
                 });
    }
    
    // Update timestamp
    book.lastUpdateTime = order.timestamp;
}

void OrderBookProcessor::processMarketOrder(const Order& order) {
    OrderBook& book = orderBooks_[order.symbol];
    double remainingQuantity = order.quantity;
    
    if (order.side == OrderSide::BUY) {
        // Match against asks
        auto it = book.asks.begin();
        while (it != book.asks.end() && remainingQuantity > 0) {
            if (it->quantity <= remainingQuantity) {
                // Fully match this level
                remainingQuantity -= it->quantity;
                it = book.asks.erase(it);
            } else {
                // Partially match this level
                it->quantity -= remainingQuantity;
                remainingQuantity = 0;
                break;
            }
        }
    } else {
        // Match against bids
        auto it = book.bids.begin();
        while (it != book.bids.end() && remainingQuantity > 0) {
            if (it->quantity <= remainingQuantity) {
                // Fully match this level
                remainingQuantity -= it->quantity;
                it = book.bids.erase(it);
            } else {
                // Partially match this level
                it->quantity -= remainingQuantity;
                remainingQuantity = 0;
                break;
            }
        }
    }
    
    // Update timestamp
    book.lastUpdateTime = order.timestamp;
    
    // Record market impact
    double fillRatio = (order.quantity - remainingQuantity) / order.quantity;
    marketImpactMetrics_[order.symbol].emplace_back(
        MarketImpact{order.timestamp, order.quantity, fillRatio, order.side});
}

void OrderBookProcessor::processCancelOrder(const Order& order) {
    // Find the original order
    auto it = activeOrders_.find(order.orderId);
    if (it == activeOrders_.end()) {
        std::cerr << "Cannot cancel order: " << order.orderId << " - not found" << std::endl;
        return;
    }
    
    const Order& originalOrder = it->second;
    OrderBook& book = orderBooks_[originalOrder.symbol];
    
    // Remove from the book
    if (originalOrder.side == OrderSide::BUY) {
        auto bidIt = std::find_if(book.bids.begin(), book.bids.end(),
                                 [&originalOrder](const OrderBookEntry& entry) {
                                     return std::abs(entry.price - originalOrder.price) < 1e-6;
                                 });
        
        if (bidIt != book.bids.end()) {
            if (bidIt->quantity <= originalOrder.quantity) {
                book.bids.erase(bidIt);
            } else {
                bidIt->quantity -= originalOrder.quantity;
            }
        }
    } else {
        auto askIt = std::find_if(book.asks.begin(), book.asks.end(),
                                 [&originalOrder](const OrderBookEntry& entry) {
                                     return std::abs(entry.price - originalOrder.price) < 1e-6;
                                 });
        
        if (askIt != book.asks.end()) {
            if (askIt->quantity <= originalOrder.quantity) {
                book.asks.erase(askIt);
            } else {
                askIt->quantity -= originalOrder.quantity;
            }
        }
    }
    
    // Remove from active orders
    activeOrders_.erase(it);
    
    // Update timestamp
    book.lastUpdateTime = order.timestamp;
}

void OrderBookProcessor::processModifyOrder(const Order& order) {
    // First cancel the original order
    processCancelOrder(order);
    
    // Then place a new order
    processLimitOrder(order);
}

void OrderBookProcessor::updateMarketMetrics(const std::string& symbol) {
    const OrderBook& book = orderBooks_[symbol];
    
    // Skip if the book is empty
    if (book.bids.empty() || book.asks.empty()) {
        return;
    }
    
    // Current time
    uint64_t now = book.lastUpdateTime;
    
    // Calculate bid-ask spread
    double bestBid = book.bids.front().price;
    double bestAsk = book.asks.front().price;
    double spread = bestAsk - bestBid;
    
    // Store spread history
    spreadHistory_[symbol].emplace_back(std::make_pair(now, spread));
    
    // Calculate mid price
    double midPrice = (bestBid + bestAsk) / 2.0;
    midPriceHistory_[symbol].emplace_back(std::make_pair(now, midPrice));
    
    // Calculate depth (total quantity at best bid and ask)
    double bidDepth = book.bids.front().quantity;
    double askDepth = book.asks.front().quantity;
    depthHistory_[symbol].emplace_back(std::make_pair(now, bidDepth));
    
    // Calculate volume imbalance
    double volumeImbalance = (bidDepth - askDepth) / (bidDepth + askDepth);
    volumeImbalanceHistory_[symbol].emplace_back(std::make_pair(now, volumeImbalance));
    
    // Calculate micro price (volume-weighted mid price)
    double microPrice = (bestBid * askDepth + bestAsk * bidDepth) / (bidDepth + askDepth);
    microPriceHistory_[symbol].emplace_back(std::make_pair(now, microPrice));
    
    // Calculate price impact for a standard order size
    double standardSize = 10000.0; // Example standard size
    double bidImpact = calculatePriceImpact(symbol, standardSize, OrderSide::BUY);
    double askImpact = calculatePriceImpact(symbol, standardSize, OrderSide::SELL);
    
    priceImpactHistory_[symbol].emplace_back(std::make_pair(now, bidImpact));
    
    // Limit history size
    const size_t MAX_HISTORY = 1000;
    if (spreadHistory_[symbol].size() > MAX_HISTORY) {
        spreadHistory_[symbol].erase(spreadHistory_[symbol].begin());
        depthHistory_[symbol].erase(depthHistory_[symbol].begin());
        volumeImbalanceHistory_[symbol].erase(volumeImbalanceHistory_[symbol].begin());
        priceImpactHistory_[symbol].erase(priceImpactHistory_[symbol].begin());
        midPriceHistory_[symbol].erase(midPriceHistory_[symbol].begin());
        microPriceHistory_[symbol].erase(microPriceHistory_[symbol].begin());
    }
}

double OrderBookProcessor::calculatePriceImpact(
    const std::string& symbol, double quantity, OrderSide side) {
    
    const OrderBook& book = orderBooks_[symbol];
    
    if (book.bids.empty() || book.asks.empty()) {
        return 0.0;
    }
    
    double remainingQty = quantity;
    double totalCost = 0.0;
    
    if (side == OrderSide::BUY) {
        // Calculate price to buy 'quantity' shares
        for (const auto& ask : book.asks) {
            double fillQty = std::min(remainingQty, ask.quantity);
            totalCost += fillQty * ask.price;
            remainingQty -= fillQty;
            
            if (remainingQty <= 0) {
                break;
            }
        }
        
        // If not enough liquidity, use the last price
        if (remainingQty > 0) {
            totalCost += remainingQty * book.asks.back().price;
        }
        
        // Calculate average price
        double avgPrice = totalCost / quantity;
        
        // Price impact: difference between average execution price and best ask
        return (avgPrice - book.asks.front().price) / book.asks.front().price;
    } else {
        // Calculate price to sell 'quantity' shares
        for (const auto& bid : book.bids) {
            double fillQty = std::min(remainingQty, bid.quantity);
            totalCost += fillQty * bid.price;
            remainingQty -= fillQty;
            
            if (remainingQty <= 0) {
                break;
            }
        }
        
        // If not enough liquidity, use the last price
        if (remainingQty > 0) {
            totalCost += remainingQty * book.bids.back().price;
        }
        
        // Calculate average price
        double avgPrice = totalCost / quantity;
        
        // Price impact: difference between best bid and average execution price
        return (book.bids.front().price - avgPrice) / book.bids.front().price;
    }
}

double OrderBookProcessor::getSpread(const std::string& symbol) const {
    const auto& history = spreadHistory_.find(symbol);
    if (history == spreadHistory_.end() || history->second.empty()) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    return history->second.back().second;
}

double OrderBookProcessor::getAverageSpread(const std::string& symbol, int lookbackPeriod) const {
    const auto& history = spreadHistory_.find(symbol);
    if (history == spreadHistory_.end() || history->second.empty()) {
        return 0.0;
    }
    
    int lookback = std::min(lookbackPeriod, static_cast<int>(history->second.size()));
    double sum = 0.0;
    auto it = history->second.end() - lookback;
    
    for (int i = 0; i < lookback; ++i) {
        sum += (it + i)->second;
    }
    
    return sum / lookback;
}

double OrderBookProcessor::getVolumeImbalance(const std::string& symbol) const {
    const auto& history = volumeImbalanceHistory_.find(symbol);
    if (history == volumeImbalanceHistory_.end() || history->second.empty()) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    return history->second.back().second;
}

double OrderBookProcessor::getMicroPrice(const std::string& symbol) const {
    const auto& history = microPriceHistory_.find(symbol);
    if (history == microPriceHistory_.end() || history->second.empty()) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    return history->second.back().second;
}

double OrderBookProcessor::getMidPrice(const std::string& symbol) const {
    const auto& history = midPriceHistory_.find(symbol);
    if (history == midPriceHistory_.end() || history->second.empty()) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    return history->second.back().second;
}

double OrderBookProcessor::getHistoricalVol(const std::string& symbol, int lookbackPeriod) const {
    const auto& history = midPriceHistory_.find(symbol);
    if (history == midPriceHistory_.end() || 
        history->second.size() < static_cast<size_t>(lookbackPeriod) + 1) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    
    // Calculate returns
    std::vector<double> returns;
    returns.reserve(lookbackPeriod);
    
    auto it = history->second.end() - lookbackPeriod - 1;
    
    for (int i = 0; i < lookbackPeriod; ++i) {
        double prev = (it + i)->second;
        double curr = (it + i + 1)->second;
        double ret = (curr - prev) / prev;
        returns.push_back(ret);
    }
    
    // Calculate standard deviation of returns
    double mean = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
    double variance = 0.0;
    
    for (double r : returns) {
        variance += (r - mean) * (r - mean);
    }
    
    variance /= returns.size();
    double stdDev = std::sqrt(variance);
    
    // Annualize (assuming returns are calculated from tick by tick data, approximate to daily)
    return stdDev * std::sqrt(252.0);
}

std::string OrderBookProcessor::getOrderBookSnapshot(const std::string& symbol, int depth) const {
    const auto& bookIt = orderBooks_.find(symbol);
    if (bookIt == orderBooks_.end()) {
        return "Order book for " + symbol + " not found.";
    }
    
    const OrderBook& book = bookIt->second;
    std::stringstream ss;
    
    ss << "Order Book for " << symbol << " at " 
       << book.lastUpdateTime << "\n";
    ss << std::setw(15) << "BID QTY" << std::setw(15) << "BID PRICE" 
       << " | " << std::setw(15) << "ASK PRICE" << std::setw(15) << "ASK QTY" << "\n";
    ss << std::string(60, '-') << "\n";
    
    // Show up to 'depth' levels
    int maxDepth = std::min(depth, 
                           std::min(static_cast<int>(book.bids.size()), 
                                   static_cast<int>(book.asks.size())));
    
    for (int i = 0; i < maxDepth; ++i) {
        if (i < static_cast<int>(book.bids.size())) {
            ss << std::setw(15) << book.bids[i].quantity << std::setw(15) << book.bids[i].price;
        } else {
            ss << std::setw(15) << " " << std::setw(15) << " ";
        }
        
        ss << " | ";
        
        if (i < static_cast<int>(book.asks.size())) {
            ss << std::setw(15) << book.asks[i].price << std::setw(15) << book.asks[i].quantity;
        } else {
            ss << std::setw(15) << " " << std::setw(15) << " ";
        }
        
        ss << "\n";
    }
    
    // Add summary metrics
    if (!book.bids.empty() && !book.asks.empty()) {
        double spread = book.asks.front().price - book.bids.front().price;
        double midPrice = (book.asks.front().price + book.bids.front().price) / 2.0;
        
        ss << "\nBid-Ask Spread: " << spread << " (" 
           << (spread / midPrice * 100.0) << "%)\n";
        ss << "Mid Price: " << midPrice << "\n";
        
        // Calculate total depth
        double totalBidQty = 0.0;
        double totalAskQty = 0.0;
        
        for (const auto& bid : book.bids) {
            totalBidQty += bid.quantity;
        }
        
        for (const auto& ask : book.asks) {
            totalAskQty += ask.quantity;
        }
        
        ss << "Total Bid Quantity: " << totalBidQty << "\n";
        ss << "Total Ask Quantity: " << totalAskQty << "\n";
        ss << "Volume Imbalance: " << (totalBidQty - totalAskQty) / (totalBidQty + totalAskQty) << "\n";
    }
    
    return ss.str();
}

std::unordered_map<std::string, double> OrderBookProcessor::analyzeOrderFlowToxicity(
    const std::string& symbol, int lookbackPeriod) const {
    
    std::unordered_map<std::string, double> metrics;
    
    const auto& marketImpactIt = marketImpactMetrics_.find(symbol);
    if (marketImpactIt == marketImpactMetrics_.end() || 
        marketImpactIt->second.size() < static_cast<size_t>(lookbackPeriod)) {
        // Not enough data
        return metrics;
    }
    
    const auto& midPriceIt = midPriceHistory_.find(symbol);
    if (midPriceIt == midPriceHistory_.end() || 
        midPriceIt->second.size() < static_cast<size_t>(lookbackPeriod)) {
        // Not enough data
        return metrics;
    }
    
    // Get the relevant history
    const auto& impacts = marketImpactIt->second;
    const auto& midPrices = midPriceIt->second;
    
    int actualLookback = std::min(lookbackPeriod, static_cast<int>(impacts.size()));
    auto impactIt = impacts.end() - actualLookback;
    
    // Calculate metrics
    double totalBuyQty = 0.0;
    double totalSellQty = 0.0;
    double buyInitiatedPriceChange = 0.0;
    double sellInitiatedPriceChange = 0.0;
    int buyOrders = 0;
    int sellOrders = 0;
    
    // Find midprice at the start of the period
    uint64_t startTime = impactIt->timestamp;
    uint64_t endTime = impacts.back().timestamp;
    
    double startMidPrice = 0.0;
    double endMidPrice = 0.0;
    
    for (const auto& [timestamp, midPrice] : midPrices) {
        if (timestamp <= startTime) {
            startMidPrice = midPrice;
        }
        if (timestamp <= endTime) {
            endMidPrice = midPrice;
        }
    }
    
    // Process each market impact event
    for (auto it = impactIt; it != impacts.end(); ++it) {
        if (it->side == OrderSide::BUY) {
            totalBuyQty += it->quantity;
            buyOrders++;
        } else {
            totalSellQty += it->quantity;
            sellOrders++;
        }
    }
    
    // Calculate price change
    double priceChange = endMidPrice - startMidPrice;
    
    // Calculate order flow imbalance
    double orderImbalance = (totalBuyQty - totalSellQty) / (totalBuyQty + totalSellQty);
    
    // Calculate price impact per unit volume
    double priceImpact = priceChange / (totalBuyQty + totalSellQty);
    
    // Calculate order toxicity index (OTI) - a measure of adverse selection
    // OTI is positive when price moves against liquidity providers
    double oti = priceChange * orderImbalance;
    
    // Store metrics
    metrics["ORDER_IMBALANCE"] = orderImbalance;
    metrics["PRICE_IMPACT"] = priceImpact;
    metrics["ORDER_TOXICITY_INDEX"] = oti;
    metrics["BUY_ORDERS"] = static_cast<double>(buyOrders);
    metrics["SELL_ORDERS"] = static_cast<double>(sellOrders);
    metrics["TOTAL_BUY_QTY"] = totalBuyQty;
    metrics["TOTAL_SELL_QTY"] = totalSellQty;
    metrics["PRICE_CHANGE"] = priceChange;
    
    return metrics;
}

std::pair<double, double> OrderBookProcessor::calculateEffectiveSpread(
    const std::string& symbol, int lookbackPeriod) const {
    
    const auto& marketImpactIt = marketImpactMetrics_.find(symbol);
    if (marketImpactIt == marketImpactMetrics_.end() || 
        marketImpactIt->second.empty()) {
        return {std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()};
    }
    
    const auto& midPriceIt = midPriceHistory_.find(symbol);
    if (midPriceIt == midPriceHistory_.end() || 
        midPriceIt->second.empty()) {
        return {std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()};
    }
    
    // Get the relevant history
    const auto& impacts = marketImpactIt->second;
    const auto& midPrices = midPriceIt->second;
    
    int actualLookback = std::min(lookbackPeriod, static_cast<int>(impacts.size()));
    if (actualLookback <= 0) {
        return {std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()};
    }
    
    auto impactIt = impacts.end() - actualLookback;
    
    double totalBuySpread = 0.0;
    double totalSellSpread = 0.0;
    int buyCount = 0;
    int sellCount = 0;
    
    for (auto it = impactIt; it != impacts.end(); ++it) {
        // Find the mid price at the time of the order
        double midPrice = 0.0;
        for (const auto& [timestamp, price] : midPrices) {
            if (timestamp <= it->timestamp) {
                midPrice = price;
            } else {
                break;
            }
        }
        
        if (midPrice <= 0.0) continue;
        
        // Estimate the execution price (this is a simplification)
        double execPrice = 0.0;
        if (it->side == OrderSide::BUY) {
            // For buys, assume the order crossed the spread and hit the ask
            const auto& book = orderBooks_.find(symbol);
            if (book != orderBooks_.end() && !book->second.asks.empty()) {
                execPrice = book->second.asks.front().price;
                totalBuySpread += 2.0 * (execPrice - midPrice) / midPrice;
                buyCount++;
            }
        } else {
            // For sells, assume the order crossed the spread and hit the bid
            const auto& book = orderBooks_.find(symbol);
            if (book != orderBooks_.end() && !book->second.bids.empty()) {
                execPrice = book->second.bids.front().price;
                totalSellSpread += 2.0 * (midPrice - execPrice) / midPrice;
                sellCount++;
            }
        }
    }
    
    double avgBuySpread = (buyCount > 0) ? totalBuySpread / buyCount : 0.0;
    double avgSellSpread = (sellCount > 0) ? totalSellSpread / sellCount : 0.0;
    
    return {avgBuySpread, avgSellSpread};
}

// Factory function implementation
std::unique_ptr<OrderBookProcessor> createOrderBookProcessor() {
    return std::make_unique<OrderBookProcessor>();
} 
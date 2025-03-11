#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <atomic>

namespace trading {

// Market data types from production system
struct MarketDataUpdate {
    std::string symbol;
    double price;
    uint64_t volume;
    std::chrono::nanoseconds timestamp;
};

// Exception hierarchy matching production
class MarketDataException : public std::runtime_error {
public:
    explicit MarketDataException(const std::string& msg) : std::runtime_error(msg) {}
};

class ConnectionException : public MarketDataException {
public:
    explicit ConnectionException(const std::string& msg) : MarketDataException(msg) {}
};

class SubscriptionException : public MarketDataException {
public:
    explicit SubscriptionException(const std::string& msg) : MarketDataException(msg) {}
};

// Market data feed interface matching production
class IMarketDataFeed {
public:
    virtual ~IMarketDataFeed() = default;
    virtual void connect() = 0;
    virtual void disconnect() = 0;
    virtual bool is_connected() const = 0;
    virtual void subscribe(const std::string& symbol) = 0;
    virtual void unsubscribe(const std::string& symbol) = 0;
};

// Market data subscription using RAII pattern from production
class MarketDataSubscription {
public:
    MarketDataSubscription(std::shared_ptr<IMarketDataFeed> feed, const std::string& symbol);
    ~MarketDataSubscription();
    
    // Prevent copying to maintain RAII semantics
    MarketDataSubscription(const MarketDataSubscription&) = delete;
    MarketDataSubscription& operator=(const MarketDataSubscription&) = delete;
    
    // Allow moving
    MarketDataSubscription(MarketDataSubscription&&) noexcept;
    MarketDataSubscription& operator=(MarketDataSubscription&&) noexcept;
    
private:
    std::weak_ptr<IMarketDataFeed> feed_;
    std::string symbol_;
};

// Market data manager class
class MarketDataManager {
public:
    // Constructor with feed configuration
    explicit MarketDataManager(const std::vector<std::shared_ptr<IMarketDataFeed>>& feeds);
    
    // Subscription management
    MarketDataSubscription subscribe(const std::string& symbol);
    void unsubscribe(const std::string& symbol);
    
    // Feed management
    void add_feed(std::shared_ptr<IMarketDataFeed> feed);
    void remove_feed(const std::shared_ptr<IMarketDataFeed>& feed);
    
    // Data access
    std::vector<MarketDataUpdate> get_updates(const std::string& symbol) const;
    
    // Connection management
    void connect_all();
    void disconnect_all();
    bool is_healthy() const;
    
    // Error handling and monitoring
    struct HealthStats {
        std::atomic<uint64_t> total_updates{0};
        std::atomic<uint64_t> error_count{0};
        std::atomic<uint64_t> reconnection_attempts{0};
        std::atomic<uint64_t> successful_reconnections{0};
        std::chrono::steady_clock::time_point last_update;
    };
    
    const HealthStats& get_health_stats() const;
    void reset_health_stats();
    
private:
    // Implementation details to be filled by students
    // following the patterns from smart_pointer_trading.cpp
    
    std::vector<std::shared_ptr<IMarketDataFeed>> feeds_;
    std::unordered_map<std::string, std::vector<std::weak_ptr<IMarketDataFeed>>> subscriptions_;
    HealthStats health_stats_;
    
    // Helper methods students need to implement
    void cleanup_expired_subscriptions();
    void handle_feed_error(const std::shared_ptr<IMarketDataFeed>& feed);
    bool attempt_reconnection(const std::shared_ptr<IMarketDataFeed>& feed);
};

} // namespace trading 
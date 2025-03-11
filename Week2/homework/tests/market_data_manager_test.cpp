#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <chrono>
#include "market_data_manager.hpp"

using namespace trading;
using namespace std::chrono_literals;

// Mock market data feed for testing
class MockMarketDataFeed : public IMarketDataFeed {
public:
    void connect() override {
        is_connected_ = true;
    }
    
    void disconnect() override {
        is_connected_ = false;
    }
    
    bool is_connected() const override {
        return is_connected_;
    }
    
    void subscribe(const std::string& symbol) override {
        if (!is_connected_) {
            throw ConnectionException("Feed not connected");
        }
        subscribed_symbols_.insert(symbol);
    }
    
    void unsubscribe(const std::string& symbol) override {
        subscribed_symbols_.erase(symbol);
    }
    
    // Test helpers
    bool is_subscribed(const std::string& symbol) const {
        return subscribed_symbols_.count(symbol) > 0;
    }
    
    void simulate_disconnect() {
        is_connected_ = false;
    }
    
private:
    bool is_connected_{false};
    std::unordered_set<std::string> subscribed_symbols_;
};

// Test fixture for market data manager tests
class MarketDataManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        feeds_.push_back(std::make_shared<MockMarketDataFeed>());
        feeds_.push_back(std::make_shared<MockMarketDataFeed>());
        manager_ = std::make_unique<MarketDataManager>(feeds_);
    }
    
    std::vector<std::shared_ptr<MockMarketDataFeed>> feeds_;
    std::unique_ptr<MarketDataManager> manager_;
};

// Test basic subscription
TEST_F(MarketDataManagerTest, BasicSubscription) {
    manager_->connect_all();
    
    auto subscription = manager_->subscribe("AAPL");
    EXPECT_TRUE(feeds_[0]->is_subscribed("AAPL"));
    EXPECT_TRUE(feeds_[1]->is_subscribed("AAPL"));
}

// Test subscription cleanup
TEST_F(MarketDataManagerTest, SubscriptionCleanup) {
    manager_->connect_all();
    
    {
        auto subscription = manager_->subscribe("AAPL");
        EXPECT_TRUE(feeds_[0]->is_subscribed("AAPL"));
    }
    
    // Subscription should be cleaned up when it goes out of scope
    EXPECT_FALSE(feeds_[0]->is_subscribed("AAPL"));
}

// Test multiple subscriptions
TEST_F(MarketDataManagerTest, MultipleSubscriptions) {
    manager_->connect_all();
    
    auto sub1 = manager_->subscribe("AAPL");
    auto sub2 = manager_->subscribe("MSFT");
    
    EXPECT_TRUE(feeds_[0]->is_subscribed("AAPL"));
    EXPECT_TRUE(feeds_[0]->is_subscribed("MSFT"));
}

// Test subscription move semantics
TEST_F(MarketDataManagerTest, SubscriptionMove) {
    manager_->connect_all();
    
    auto sub1 = manager_->subscribe("AAPL");
    auto sub2 = std::move(sub1);
    
    EXPECT_TRUE(feeds_[0]->is_subscribed("AAPL"));
    
    sub2 = MarketDataSubscription(std::move(sub2));
    EXPECT_TRUE(feeds_[0]->is_subscribed("AAPL"));
}

// Test feed management
TEST_F(MarketDataManagerTest, FeedManagement) {
    auto new_feed = std::make_shared<MockMarketDataFeed>();
    manager_->add_feed(new_feed);
    
    manager_->connect_all();
    auto subscription = manager_->subscribe("AAPL");
    
    EXPECT_TRUE(new_feed->is_subscribed("AAPL"));
    
    manager_->remove_feed(new_feed);
    EXPECT_FALSE(new_feed->is_subscribed("AAPL"));
}

// Test error handling
TEST_F(MarketDataManagerTest, ErrorHandling) {
    manager_->connect_all();
    
    // Simulate a feed disconnection
    feeds_[0]->simulate_disconnect();
    
    EXPECT_THROW({
        manager_->subscribe("AAPL");
    }, ConnectionException);
    
    const auto& stats = manager_->get_health_stats();
    EXPECT_GT(stats.error_count.load(), 0);
}

// Test reconnection
TEST_F(MarketDataManagerTest, Reconnection) {
    manager_->connect_all();
    auto subscription = manager_->subscribe("AAPL");
    
    // Simulate a feed disconnection
    feeds_[0]->simulate_disconnect();
    EXPECT_FALSE(feeds_[0]->is_connected());
    
    // Manager should attempt to reconnect
    manager_->connect_all();
    EXPECT_TRUE(feeds_[0]->is_connected());
    EXPECT_TRUE(feeds_[0]->is_subscribed("AAPL"));
    
    const auto& stats = manager_->get_health_stats();
    EXPECT_GT(stats.reconnection_attempts.load(), 0);
    EXPECT_GT(stats.successful_reconnections.load(), 0);
}

// Test health monitoring
TEST_F(MarketDataManagerTest, HealthMonitoring) {
    manager_->connect_all();
    
    const auto& initial_stats = manager_->get_health_stats();
    auto initial_updates = initial_stats.total_updates.load();
    
    auto subscription = manager_->subscribe("AAPL");
    auto updates = manager_->get_updates("AAPL");
    
    const auto& current_stats = manager_->get_health_stats();
    EXPECT_GT(current_stats.total_updates.load(), initial_updates);
    EXPECT_GT(current_stats.last_update, initial_stats.last_update);
}

// Test concurrent operations
TEST_F(MarketDataManagerTest, ConcurrentOperations) {
    manager_->connect_all();
    
    static constexpr size_t NUM_THREADS = 4;
    static constexpr size_t OPS_PER_THREAD = 100;
    
    std::vector<std::thread> threads;
    std::atomic<size_t> successful_ops{0};
    
    for (size_t i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([this, &successful_ops]() {
            for (size_t j = 0; j < OPS_PER_THREAD; ++j) {
                try {
                    auto symbol = "SYM" + std::to_string(j);
                    auto subscription = manager_->subscribe(symbol);
                    auto updates = manager_->get_updates(symbol);
                    successful_ops.fetch_add(1);
                } catch (const MarketDataException&) {
                    // Expected when feeds are disconnected
                }
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_GT(successful_ops.load(), 0);
}

// Test system health check
TEST_F(MarketDataManagerTest, SystemHealth) {
    manager_->connect_all();
    EXPECT_TRUE(manager_->is_healthy());
    
    // Simulate all feeds disconnecting
    for (auto& feed : feeds_) {
        feed->simulate_disconnect();
    }
    
    EXPECT_FALSE(manager_->is_healthy());
}

// Test stats reset
TEST_F(MarketDataManagerTest, StatsReset) {
    manager_->connect_all();
    auto subscription = manager_->subscribe("AAPL");
    
    const auto& stats = manager_->get_health_stats();
    EXPECT_GT(stats.total_updates.load(), 0);
    
    manager_->reset_health_stats();
    EXPECT_EQ(0u, stats.total_updates.load());
    EXPECT_EQ(0u, stats.error_count.load());
    EXPECT_EQ(0u, stats.reconnection_attempts.load());
} 
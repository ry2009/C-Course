#include <gtest/gtest.h>
#include "../src/market_data_handler.hpp"
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <atomic>

using namespace trading;
using namespace std::chrono_literals;

class MarketDataHandlerTest : public ::testing::Test {
protected:
    void SetUp() override {
        handler = std::make_unique<MarketDataHandler>(100); // Up to 100 symbols
    }

    void TearDown() override {
        handler.reset();
    }

    std::unique_ptr<MarketDataHandler> handler;
};

// Test adding exchanges
TEST_F(MarketDataHandlerTest, AddExchanges) {
    EXPECT_TRUE(handler->add_exchange("NYSE"));
    EXPECT_TRUE(handler->add_exchange("NASDAQ"));
    EXPECT_TRUE(handler->add_exchange("CME"));
}

// Test subscribing to market data
TEST_F(MarketDataHandlerTest, SubscribeToMarketData) {
    std::atomic<int> update_count(0);
    
    auto callback = [&update_count](const MarketUpdate& update) {
        update_count++;
    };
    
    EXPECT_TRUE(handler->subscribe("AAPL", callback));
    EXPECT_TRUE(handler->subscribe("MSFT", callback));
    EXPECT_TRUE(handler->subscribe("GOOG", callback));
}

// Test processing market updates
TEST_F(MarketDataHandlerTest, ProcessMarketUpdates) {
    std::atomic<int> aapl_updates(0);
    std::atomic<int> msft_updates(0);
    
    auto aapl_callback = [&aapl_updates](const MarketUpdate& update) {
        aapl_updates++;
    };
    
    auto msft_callback = [&msft_updates](const MarketUpdate& update) {
        msft_updates++;
    };
    
    // Add exchanges and subscribe to symbols
    ASSERT_TRUE(handler->add_exchange("NYSE"));
    ASSERT_TRUE(handler->subscribe("AAPL", aapl_callback));
    ASSERT_TRUE(handler->subscribe("MSFT", msft_callback));
    
    // Start processing
    handler->start();
    
    // Create market updates manually
    MarketUpdate aapl_update;
    aapl_update.symbol = "AAPL";
    aapl_update.exchange = "NYSE";
    aapl_update.bid_price = 150.0;
    aapl_update.ask_price = 150.1;
    aapl_update.last_price = 150.05;
    aapl_update.volume = 100;
    aapl_update.timestamp = std::chrono::steady_clock::now().time_since_epoch();
    
    MarketUpdate msft_update;
    msft_update.symbol = "MSFT";
    msft_update.exchange = "NYSE";
    msft_update.bid_price = 250.0;
    msft_update.ask_price = 250.1;
    msft_update.last_price = 250.05;
    msft_update.volume = 200;
    msft_update.timestamp = std::chrono::steady_clock::now().time_since_epoch();
    
    // Process updates
    handler->process_update(aapl_update);
    handler->process_update(msft_update);
    handler->process_update(aapl_update);
    
    // Allow time for processing
    std::this_thread::sleep_for(100ms);
    
    // Stop processing
    handler->stop();
    
    // Check that updates were processed
    EXPECT_EQ(aapl_updates, 2);
    EXPECT_EQ(msft_updates, 1);
}

// Test concurrent market updates from multiple exchanges
TEST_F(MarketDataHandlerTest, ConcurrentUpdates) {
    std::atomic<int> update_count(0);
    
    auto callback = [&update_count](const MarketUpdate& update) {
        update_count++;
    };
    
    // Add exchanges and subscribe to symbols
    ASSERT_TRUE(handler->add_exchange("NYSE"));
    ASSERT_TRUE(handler->add_exchange("NASDAQ"));
    ASSERT_TRUE(handler->add_exchange("CME"));
    ASSERT_TRUE(handler->subscribe("AAPL", callback));
    
    // Start processing
    handler->start();
    
    // Create multiple threads to send updates concurrently
    std::vector<std::thread> threads;
    const int num_threads = 3;
    const int updates_per_thread = 100;
    
    for (int i = 0; i < num_threads; i++) {
        std::string exchange = (i == 0) ? "NYSE" : ((i == 1) ? "NASDAQ" : "CME");
        
        threads.emplace_back([this, exchange, updates_per_thread]() {
            for (int j = 0; j < updates_per_thread; j++) {
                MarketUpdate update;
                update.symbol = "AAPL";
                update.exchange = exchange;
                update.bid_price = 150.0 + (j * 0.01);
                update.ask_price = 150.1 + (j * 0.01);
                update.last_price = 150.05 + (j * 0.01);
                update.volume = 100 + j;
                update.timestamp = std::chrono::steady_clock::now().time_since_epoch();
                
                handler->process_update(update);
                
                // Small delay to simulate real-world scenario
                std::this_thread::sleep_for(1ms);
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }
    
    // Allow time for processing all updates
    std::this_thread::sleep_for(100ms);
    
    // Stop processing
    handler->stop();
    
    // Check that updates were processed
    EXPECT_EQ(update_count, num_threads * updates_per_thread);
    
    // Check metrics
    MarketDataMetrics metrics = handler->get_metrics();
    EXPECT_EQ(metrics.total_updates_processed, num_threads * updates_per_thread);
    
    // Should have metrics for each exchange
    EXPECT_TRUE(metrics.avg_latency_us.find("NYSE") != metrics.avg_latency_us.end());
    EXPECT_TRUE(metrics.avg_latency_us.find("NASDAQ") != metrics.avg_latency_us.end());
    EXPECT_TRUE(metrics.avg_latency_us.find("CME") != metrics.avg_latency_us.end());
}

// Test order book updates
TEST_F(MarketDataHandlerTest, OrderBookUpdates) {
    // Add exchange and subscribe to symbol
    ASSERT_TRUE(handler->add_exchange("NYSE"));
    
    // Start processing
    handler->start();
    
    // Create market updates for order book
    MarketUpdate update;
    update.symbol = "AAPL";
    update.exchange = "NYSE";
    update.bid_price = 150.0;
    update.ask_price = 150.1;
    update.last_price = 150.05;
    update.volume = 100;
    update.timestamp = std::chrono::steady_clock::now().time_since_epoch();
    
    // Process update
    handler->process_update(update);
    
    // Allow time for processing
    std::this_thread::sleep_for(50ms);
    
    // Get order book
    OrderBook book = handler->get_order_book("AAPL");
    
    // Check order book
    EXPECT_EQ(book.symbol, "AAPL");
    EXPECT_FALSE(book.bids.empty());
    EXPECT_FALSE(book.asks.empty());
    
    // Check bid/ask prices
    if (!book.bids.empty() && !book.asks.empty()) {
        EXPECT_DOUBLE_EQ(book.bids[0].price, 150.0);
        EXPECT_DOUBLE_EQ(book.asks[0].price, 150.1);
    }
    
    // Stop processing
    handler->stop();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 
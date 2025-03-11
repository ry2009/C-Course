CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -O2 -pthread
INCLUDES = -I./include
LIBS = -lgtest -lpthread

# Source and object files
SRC_DIR = src
TEST_DIR = tests
OBJ_DIR = obj
BIN_DIR = bin

# Create directories if they don't exist
$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR))

# Source files
ORDER_BOOK_SRC = $(SRC_DIR)/order_book_allocator.cpp
MARKET_DATA_SRC = $(SRC_DIR)/market_data_manager.cpp
TIME_SERIES_SRC = $(SRC_DIR)/time_series_pool.cpp
LOCK_FREE_SRC = $(SRC_DIR)/lock_free_components.cpp

# Test files
ORDER_BOOK_TEST = $(TEST_DIR)/order_book_allocator_test.cpp
MARKET_DATA_TEST = $(TEST_DIR)/market_data_manager_test.cpp
TIME_SERIES_TEST = $(TEST_DIR)/time_series_pool_test.cpp
LOCK_FREE_TEST = $(TEST_DIR)/lock_free_components_test.cpp

# Targets
.PHONY: all clean tests

all: tests

# Test targets
tests: $(BIN_DIR)/test_order_book $(BIN_DIR)/test_market_data $(BIN_DIR)/test_time_series $(BIN_DIR)/test_lock_free

$(BIN_DIR)/test_order_book: $(ORDER_BOOK_TEST) $(ORDER_BOOK_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

$(BIN_DIR)/test_market_data: $(MARKET_DATA_TEST) $(MARKET_DATA_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

$(BIN_DIR)/test_time_series: $(TIME_SERIES_TEST) $(TIME_SERIES_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

$(BIN_DIR)/test_lock_free: $(LOCK_FREE_TEST) $(LOCK_FREE_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

# Run tests
run_tests: tests
	$(BIN_DIR)/test_order_book
	$(BIN_DIR)/test_market_data
	$(BIN_DIR)/test_time_series
	$(BIN_DIR)/test_lock_free

# Clean up
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) 
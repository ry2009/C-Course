cmake_minimum_required(VERSION 3.10)
project(OrderBookProcessor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(order_book_test 
    src/order_book_processor.cpp
    src/order_book_test.cpp)

target_include_directories(order_book_test PRIVATE src) 
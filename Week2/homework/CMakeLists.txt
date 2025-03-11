cmake_minimum_required(VERSION 3.10)
project(Week2_Homework)

# Set C++ standard to C++17 as used in production
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compiler warnings based on compiler
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror -Wpedantic)
endif()

# Enable testing
enable_testing()

# Find required packages
find_package(Threads REQUIRED)
find_package(GTest REQUIRED)

# Source files
set(SOURCES
    src/order_book_allocator.cpp
    src/market_data_manager.cpp
    src/time_series_pool.cpp
    src/lock_free_components.cpp
)

# Header files
set(HEADERS
    include/order_book_allocator.hpp
    include/market_data_manager.hpp
    include/time_series_pool.hpp
    include/lock_free_components.hpp
)

# Create homework solution executable
add_executable(homework_solution ${SOURCES} ${HEADERS})
target_include_directories(homework_solution PRIVATE include)
target_link_libraries(homework_solution PRIVATE Threads::Threads)

# Set output directories
set_target_properties(homework_solution
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Add test executable
add_executable(homework_tests
    tests/order_book_allocator_test.cpp
    tests/market_data_manager_test.cpp
    tests/time_series_pool_test.cpp
    tests/lock_free_components_test.cpp
)
target_include_directories(homework_tests PRIVATE include)
target_link_libraries(homework_tests PRIVATE
    GTest::GTest
    GTest::Main
    Threads::Threads
)

# Add benchmark executable
find_package(benchmark REQUIRED)
add_executable(homework_benchmarks
    benchmarks/order_book_allocator_bench.cpp
    benchmarks/market_data_manager_bench.cpp
    benchmarks/time_series_pool_bench.cpp
    benchmarks/lock_free_components_bench.cpp
)
target_include_directories(homework_benchmarks PRIVATE include)
target_link_libraries(homework_benchmarks PRIVATE
    benchmark::benchmark
    Threads::Threads
)

# Add tests
add_test(NAME homework_tests COMMAND homework_tests)

# Documentation
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()

# Installation
install(TARGETS homework_solution
    RUNTIME DESTINATION bin
)

install(FILES ${HEADERS}
    DESTINATION include/trading
) 
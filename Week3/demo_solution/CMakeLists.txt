cmake_minimum_required(VERSION 3.10)
project(TradingSystemDemo VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compiler flags
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Output build type
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ flags: ${CMAKE_CXX_FLAGS}")

# Detect compiler
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    message(STATUS "C++ compiler: GCC")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(STATUS "C++ compiler: AppleClang")
elseif(MSVC)
    message(STATUS "C++ compiler: MSVC")
endif()

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/market_data_handler.cpp
    src/thread_pool.cpp
)

# Create static library
add_library(trading_lib STATIC ${SOURCES})

# Add the integrated system test executable
add_executable(integrated_system_test src/integrated_system_test.cpp)
target_link_libraries(integrated_system_test trading_lib)

# Find threads package and link against it
find_package(Threads REQUIRED)
target_link_libraries(trading_lib Threads::Threads)
target_link_libraries(integrated_system_test Threads::Threads)

# Print some info
message(STATUS "Source files: ${SOURCES}")
message(STATUS "Executables: integrated_system_test")

# Installation targets
install(TARGETS trading_lib DESTINATION lib)
install(TARGETS integrated_system_test DESTINATION bin)
install(DIRECTORY include/ DESTINATION include)

# Enable testing
enable_testing()
add_test(NAME integrated_system_test COMMAND integrated_system_test) 
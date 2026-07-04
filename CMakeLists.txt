cmake_minimum_required(VERSION 3.16)
project(OrderBook LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

add_library(orderbook
    OrderBook/Order.cpp
    OrderBook/OrderBook.cpp
    Engine/BoundedQueue.cpp
    Engine/Engine.cpp
    OrderGenerator/MarkovParetoOrderGenerator.cpp
)
target_include_directories(orderbook PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(orderbook PUBLIC Threads::Threads)

if(MSVC)
    target_compile_options(orderbook PRIVATE /W4)
else()
    target_compile_options(orderbook PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(orderbook_demo main.cpp)
target_link_libraries(orderbook_demo PRIVATE orderbook)

enable_testing()
add_executable(orderbook_tests tests.cpp)
target_link_libraries(orderbook_tests PRIVATE orderbook)
add_test(NAME orderbook_tests COMMAND orderbook_tests)

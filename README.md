# OrderBook

A small C++ order-matching engine project.

## Overview
This project implements a basic order book with:
- BUY and SELL orders
- order creation, modification, and cancellation
- price-level matching
- a simple command-driven engine

## Project Structure
- `OrderBook/` - core order book and order logic
- `Engine/` - command queue and matching engine

## Build
Compile the project with a C++17 compiler, for example:

```sh
g++ -std=c++17 -Wall -Wextra -I. \
  OrderBook/OrderBook.cpp \
  OrderBook/Order.cpp \
  Engine/Engine.cpp \
  Engine/BoundedQueue.cpp \
  -o orderbook_demo
```

## Notes
This is a learning / prototype implementation of an order book matching flow.

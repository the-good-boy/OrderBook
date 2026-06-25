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

## Build and run

The project requires a C++20 compiler.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/orderbook_demo
```

On Windows with a multi-config generator, the executable may be under
`build/Debug/`.

## Design

- Bid prices are stored descending and ask prices ascending, making the best
  prices available at the beginning of each map.
- Each price level uses a linked list to preserve FIFO priority.
- An order-id index and per-level iterator index make lookup and cancellation
  constant-time on average.
- A bounded blocking queue serializes commands through one engine thread,
  avoiding locks inside the order book itself.

This is a learning/prototype implementation. Prices are integer ticks rather
than floating-point values. The matcher reports the bid and ask that crossed;
a production version should also retain arrival metadata so it can report the
canonical resting-price execution and richer audit events.

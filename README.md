# OrderBook

A C++20 limit order book and single-consumer matching engine with synthetic
order-flow generation and queue benchmarks.

## Overview
This project implements a basic order book with:
- BUY and SELL orders
- order creation, modification, and cancellation
- price-level matching
- a command-driven engine with pluggable queue implementations
- synthetic Markov/Pareto order generation for repeatable benchmarks

## Project Structure
- `OrderBook/` - core order book and order logic
- `Engine/` - command queues and matching engine
- `OrderGenerator/` - synthetic order-flow generator

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

## Benchmark

`main.cpp` compares two engine queue implementations:

- `BoundedQueue`: mutex + condition-variable bounded blocking queue
- `MPSCQueue`: lockless multi-producer, single-consumer queue

The benchmark uses 4 producer threads submitting commands into one engine
consumer. Each workload processes 5,000,000 operations and reports the average
of 3 runs.

Latest local result:

```text
insertions: queue=bounded ops=5000000 producers=4 avg_seconds=32.97 throughput=151663/sec
mixed: queue=bounded ops=5000000 producers=4 avg_seconds=35.27 throughput=141773/sec
insertions: queue=mpsc ops=5000000 producers=4 avg_seconds=8.88 throughput=562784/sec
mixed: queue=mpsc ops=5000000 producers=4 avg_seconds=7.95 throughput=629307/sec
```

In this workload, the lockless MPSC queue significantly reduces producer-side
contention compared with the mutex-backed bounded queue. Results vary by
machine, compiler, build type, and background load.

## Design

- Bid prices are stored descending and ask prices ascending, making the best
  prices available at the beginning of each map.
- Each price level uses a linked list to preserve FIFO priority.
- An order-id index and per-level iterator index make lookup and cancellation
  constant-time on average.
- Commands are submitted by producers and serialized through one engine
  consumer thread, avoiding locks inside the order book itself.
- The engine can use either the bounded blocking queue or the lockless MPSC
  queue through the same command-processing path.

This is a learning/prototype implementation. Prices are integer ticks rather
than floating-point values. The matcher reports the bid and ask that crossed;
a production version should also retain arrival metadata so it can report the
canonical resting-price execution and richer audit events.

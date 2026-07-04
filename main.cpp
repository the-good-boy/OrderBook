#include "Engine/Engine.h"
#include "OrderGenerator/MarkovParetoOrderGenerator.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

namespace {

constexpr std::size_t N = 5'000'000;
constexpr std::size_t Runs = 3;
constexpr std::size_t Producers = 4;
constexpr Price ReferencePrice = 10'000;

const TransitionMatrix Matrix = {
    {0.80, 0.10, 0.10},
    {0.15, 0.75, 0.10},
    {0.15, 0.10, 0.75},
};

Command generatedAdd(OrderId id, OrderGenerator& generator) {
    generator.nextState();
    const Side side = generator.pickOrderSide();

    return {
        CommandType::ADD,
        0,
        id,
        side,
        generator.generateOrderPrice(ReferencePrice, side),
        generator.generateOrderSize(),
    };
}

template <typename EngineType>
void waitFor(EngineType& engine, SeqNum seq) {
    while (engine.lastProcessed() < seq) {
        std::this_thread::yield();
    }
}

std::size_t workFor(std::size_t producer) {
    return N / Producers + (producer < N % Producers ? 1 : 0);
}

OrderId firstIdFor(std::size_t producer) {
    OrderId id = 1;
    for (std::size_t i = 0; i < producer; ++i) {
        id += static_cast<OrderId>(workFor(i));
    }
    return id;
}

template <typename EngineType>
double insertionBenchmark() {
    EngineType engine;
    engine.start();
    std::vector<std::thread> producers;
    std::vector<SeqNum> lastSeqs(Producers);

    const auto start = std::chrono::steady_clock::now();
    for (std::size_t p = 0; p < Producers; ++p) {
        producers.emplace_back([&, p] {
            std::mt19937 rng(42 + static_cast<std::uint32_t>(p));
            OrderGenerator generator(Matrix, rng);
            OrderId id = firstIdFor(p);

            for (std::size_t i = 0; i < workFor(p); ++i) {
                lastSeqs[p] = engine.submit(generatedAdd(id++, generator));
            }
        });
    }
    for (auto& producer : producers) {
        producer.join();
    }

    SeqNum lastSeq = 0;
    for (const SeqNum seq : lastSeqs) {
        if (seq > lastSeq) {
            lastSeq = seq;
        }
    }
    waitFor(engine, lastSeq);
    engine.stop();

    return std::chrono::duration<double>(
               std::chrono::steady_clock::now() - start)
        .count();
}

template <typename EngineType>
double mixedBenchmark() {
    EngineType engine;
    engine.start();
    std::vector<std::thread> producers;
    std::vector<SeqNum> lastSeqs(Producers);

    const auto start = std::chrono::steady_clock::now();
    for (std::size_t p = 0; p < Producers; ++p) {
        producers.emplace_back([&, p] {
            std::mt19937 rng(42 + static_cast<std::uint32_t>(p));
            OrderGenerator generator(Matrix, rng);
            std::vector<OrderId> active;
            active.reserve(workFor(p));
            OrderId nextId = firstIdFor(p);

            for (std::size_t i = 0; i < workFor(p); ++i) {
                if (active.empty() || i % 4 < 2) {
                    generator.nextState();
                    lastSeqs[p] = engine.submit({
                        CommandType::ADD,
                        0,
                        nextId,
                        Side::BUY,
                        generator.generateOrderPrice(ReferencePrice, Side::BUY),
                        generator.generateOrderSize(),
                    });
                    active.push_back(nextId++);
                } else if (i % 4 == 2) {
                    lastSeqs[p] = engine.submit({
                        CommandType::MODIFY,
                        0,
                        active.back(),
                        Side::BUY,
                        ReferencePrice + static_cast<Price>(i % 100),
                        10,
                    });
                } else {
                    lastSeqs[p] = engine.submit({
                        CommandType::CANCEL,
                        0,
                        active.back(),
                        Side::BUY,
                        0,
                        0,
                    });
                    active.pop_back();
                }
            }
        });
    }
    for (auto& producer : producers) {
        producer.join();
    }

    SeqNum lastSeq = 0;
    for (const SeqNum seq : lastSeqs) {
        if (seq > lastSeq) {
            lastSeq = seq;
        }
    }
    waitFor(engine, lastSeq);
    engine.stop();

    return std::chrono::duration<double>(
               std::chrono::steady_clock::now() - start)
        .count();
}

template <typename Benchmark>
double average(Benchmark benchmark) {
    double seconds = 0.0;
    for (std::size_t i = 0; i < Runs; ++i) {
        seconds += benchmark();
    }
    return seconds / static_cast<double>(Runs);
}

void printResult(const char* name, const char* queue, double seconds) {
    std::cout << name
              << ": queue=" << queue
              << " ops=" << N
              << " producers=" << Producers
              << " avg_seconds=" << std::fixed << std::setprecision(2) << seconds
              << " throughput=" << static_cast<std::uint64_t>(N / seconds)
              << "/sec\n";
}

} // namespace

int main() {
    printResult("insertions", "bounded", average(insertionBenchmark<Engine>));
    printResult("mixed", "bounded", average(mixedBenchmark<Engine>));
    printResult("insertions", "mpsc", average(insertionBenchmark<MPSCEngine>));
    printResult("mixed", "mpsc", average(mixedBenchmark<MPSCEngine>));
}

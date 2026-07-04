#pragma once
#include "Command.h"
#include "BoundedQueue.h"
#include "MPSCQueue.h"
#include "../OrderBook/OrderBook.h"
#include <atomic>
#include <functional>
#include <memory>
#include <stdexcept>
#include <thread>
#include <utility>

using ErrorHandler = std::function<void(const Command&, const std::exception&)>;

template <typename Queue>
class EngineImpl {
private:
    std::atomic<bool> running;
    std::thread engineThread;

    std::atomic<SeqNum> nextSeq;
    std::atomic<SeqNum> lastAppliedSeq;
    Queue queue;
    OrderBook book;

    ErrorHandler onError;

    /**
     * @brief Apply a single command to the order book
     * @param cmd Command to apply
     */
    void apply(const Command& cmd) {
        switch (cmd.type) {
            case CommandType::ADD: {
                const OrderPointer order =
                    std::make_shared<Order>(cmd.orderId, cmd.side, cmd.price, cmd.qty);
                book.addOrder(order);
                break;
            }
            case CommandType::MODIFY: {
                book.modifyOrder(cmd.orderId, cmd.price, cmd.qty);
                break;
            }
            case CommandType::CANCEL: {
                book.cancelOrder(cmd.orderId);
                break;
            }
        }
    }

    /**
     * @brief Runs the engine loop that consumes commands from the queue
     */
    void run() {
        while (running.load(std::memory_order_acquire) || !queue.isEmpty()) {
            Command cmd;
            if (!queue.pop(cmd)) {
                std::this_thread::yield();
                continue;
            }

            try {
                apply(cmd);
                book.matchOrders();
                lastAppliedSeq.store(cmd.seqNum, std::memory_order_release);
            } catch (const std::exception& e) {
                if (onError) { onError(cmd, e); }
                lastAppliedSeq.store(cmd.seqNum, std::memory_order_release);
            }
        }
    }

public:
    /**
     * @brief Construct the engine
     * @param queueCapacity Capacity of the internal command queue
     */
    explicit EngineImpl(size_t queueCapacity = 1 << 16):
        running(false),
        nextSeq(1),
        lastAppliedSeq(0),
        queue(queueCapacity)
    {}

    /**
     * @brief Destroy the engine, stopping the worker thread if running
     */
    ~EngineImpl() { stop(); }

    /**
     * @brief Start the worker thread
     */
    void start() {
        bool expected = false;
        if (!running.compare_exchange_strong(expected, true)) { return; }
        engineThread = std::thread([this]{ this->run(); });
    }

    /**
     * @brief Stop the worker thread and flush the queue
     */
    void stop() {
        bool expected = true;
        if (!running.compare_exchange_strong(expected, false)) { return; }
        queue.stop();
        if (engineThread.joinable()) { engineThread.join(); }
    }

    /**
     * @brief Submit a command to the engine
     * @param cmd Command to execute
     * @return Sequence number assigned to the command
     */
    SeqNum submit(Command cmd) {
        if (!running.load(std::memory_order_acquire)) {
            throw std::logic_error("Engine must be running before commands can be submitted");
        }
        cmd.seqNum = nextSeq.fetch_add(1, std::memory_order_relaxed);
        if (!queue.push(cmd)) {
            throw std::logic_error("Cannot submit to a stopped engine");
        }
        return cmd.seqNum;
    }

    /**
     * @brief Get a const reference to the internal order book
     * @return Reference to the order book
     */
    const OrderBook& getBook() const { return book; }

    /**
     * @brief Get the sequence number of the last processed command
     * @return Last applied sequence number
     */
    SeqNum lastProcessed() const { return lastAppliedSeq.load(); }

    /**
     * @brief Set a custom error handler for failed commands
     * @param f Function taking the command and the thrown exception
     */
    void set_error_handler(ErrorHandler f) { onError = std::move(f); }
};

using Engine = EngineImpl<BoundedQueue>;
using MPSCEngine = EngineImpl<MPSCQueue>;

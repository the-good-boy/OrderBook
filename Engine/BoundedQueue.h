#pragma once
#include "Command.h"
#include <condition_variable>
#include <deque>
#include <mutex>
#include <utility>


class BoundedQueue {
private:
    size_t capacity;
    std::deque<Command> q;
    mutable std::mutex m;
    std::condition_variable notFull, notEmpty;
    bool stopped;

public:
    explicit BoundedQueue(size_t capacity);

    bool push(const Command& command);

    bool pop(Command& command);

    bool isEmpty() const;

    void stop();
};

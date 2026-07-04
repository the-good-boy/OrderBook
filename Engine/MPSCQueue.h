#pragma once

#include "Command.h"

#include <atomic>
#include <cstddef>

class MPSCQueue {
private:
    struct Node {
        explicit Node(const Command& command): command(command) {}

        Command command{};
        std::atomic<Node*> next{nullptr};
    };

    std::atomic<Node*> head;
    Node* tail;
    std::atomic<bool> stopped;

public:
    explicit MPSCQueue(size_t): head(new Node({})), tail(head.load()), stopped(false) {}

    ~MPSCQueue() {
        stop();
        Command command;
        while (pop(command)) {}
        delete tail;
    }

    MPSCQueue(const MPSCQueue&) = delete;
    MPSCQueue& operator=(const MPSCQueue&) = delete;

    bool push(const Command& command) {
        if (stopped.load(std::memory_order_acquire)) {
            return false;
        }

        Node* node = new Node(command);
        Node* previous = head.exchange(node, std::memory_order_acq_rel);
        previous->next.store(node, std::memory_order_release);
        return true;
    }

    bool pop(Command& command) {
        Node* next = tail->next.load(std::memory_order_acquire);
        if (!next) {
            return false;
        }

        command = next->command;
        delete tail;
        tail = next;
        return true;
    }

    bool isEmpty() const {
        return tail->next.load(std::memory_order_acquire) == nullptr;
    }

    void stop() {
        stopped.store(true, std::memory_order_release);
    }
};

#pragma once
#include "../OrderBook/Order.h"

enum class CommandType{
    ADD,
    MODIFY,
    CANCEL
};

using SeqNum = std::uint64_t;

struct Command{
    CommandType type;
    SeqNum seqNum;

    OrderId orderId;
    Side side;
    Price price;
    Quantity qty;
};
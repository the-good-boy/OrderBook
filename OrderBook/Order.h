#pragma once
#include <iostream>

enum class Side{
    BUY,
    SELL
};

enum class Status{
    PENDING,
    PARTIALLY_FILLED,
    COMPLETELY_FILLED
};

using OrderId = std::uint64_t;
using Price = std::uint32_t;
using Quantity = std::uint32_t;

class Order{
private:
    OrderId orderId;
    Side side;
    Price price;
    Quantity initialQuantity;
    Quantity remainingQuantity;
    Status status;
public:
    Order(OrderId id, Side side, Price price, Quantity qty);

    OrderId getOrderId() const;

    Side getSide() const;

    Price getPrice() const;

    Quantity getInitialQuantity() const;

    Quantity getRemainingQuantity() const;

    Quantity getFilledQuantity() const;

    Status getStatus() const;

    void fill(std::uint32_t qty);
};

using OrderPointer = std::shared_ptr<Order>;
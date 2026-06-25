#pragma once
#include "Order.h"
#include <list>
#include <map>
#include <optional>
#include <unordered_map>
#include <vector>

struct PriceLevel{
    Price price;
    std::list<OrderPointer> orders;
    std::unordered_map<OrderId, std::list<OrderPointer>::iterator> orderIters;
};


using PriceLevelPointer = std::shared_ptr<PriceLevel>;

struct Trade {
    OrderId buyOrderId;
    OrderId sellOrderId;
    Price bidPrice;
    Price askPrice;
    Quantity quantity;
};

class OrderBook {
private:
    std::map<Price, PriceLevelPointer, std::greater<>>bids;
    std::map<Price, PriceLevelPointer> asks;
    std::unordered_map<OrderId, OrderPointer> orders;
    std::vector<Trade> trades;

public:

    void addOrder(const OrderPointer& order);

    OrderPointer modifyOrder(OrderId orderId, Price newPrice, Quantity newQty);

    void cancelOrder(OrderId orderId);

    void matchOrders();

    OrderPointer getOrderById(OrderId orderId) const;

    std::size_t getNumberOfOrders() const;

    bool contains(OrderId orderId) const;

    std::optional<Price> getBestBid() const;

    std::optional<Price> getBestAsk() const;

    const std::vector<Trade>& getTrades() const;
};

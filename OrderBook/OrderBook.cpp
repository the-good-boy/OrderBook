#include "OrderBook.h"

void OrderBook::addOrder(const OrderPointer& order) {
    switch(order->getSide()) {
        case Side::BUY: {
            if(!bids.contains(order->getPrice())) {
                PriceLevelPointer newPriceLevel = std::make_shared<PriceLevel>();
                newPriceLevel->price = order->getPrice();
                bids.insert(std::make_pair(order->getPrice(), newPriceLevel));
            }

            const PriceLevelPointer priceLevel = bids[order->getPrice()];
            const auto it = priceLevel->orders.append(priceLevel->orders.end(), order);
            priceLevel->orderIters[order->getOrderId] = it;
            break;
        }
        case Side::SELL: {
            if(!asks.contains(order->getPrice())) {
                PriceLevelPointer newPriceLevel = std::make_shared<PriceLevel>();
                newPriceLevel->price = order->getPrice();
                asks.insert(std::make_pair(order->getPrice(), newPriceLevel));
            }

            const PriceLevelPointer priceLevel = asks[order->getPrice()];
            const auto it = priceLevel->orders.append(priceLevel->orders.end(), order);
            priceLevel->orderIters[order->getOrderId] = it;
            break;
        }
    }

    orders[order->getOrderId()] = order;
}

OrderPointer OrderBook::modifyOrder(OrderId orderId, Price newPrice, Quantity newQty) {
    if (!orders.contains(orderId)) {
        throw std::logic_error("Order('" + std::to_string(orderId) + "') does not exist");
    }
    const OrderPointer tempOrder = orders[orderId];
    if (tempOrder->getStatus() != Status::PENDING) {
        throw std::logic_error("Order('" + std::to_string(orderId) + "') is not pending so it cannot be modified");
    }

    const OrderPointer order = std::make_shared<Order>(tempOrder->getOrderId(), tempOrder->getSide(), newPrice, newQty);
    cancelOrder(orderId);
    addOrder(order);
    return order;
}

void OrderBook::cancelOrder(const OrderId orderId) {
    if (!orders.contains(orderId)) {
        throw std::logic_error("Order ('" + std::to_string(orderId) + "') does not exist or was completely filled");
    }
    const OrderPointer order = orders[orderId];
    if (order->getStatus() == Status::PARTIALLY_FILLED) {
        throw std::logic_error("Order ('" + std::to_string(orderId) + "') cannot be canceled as it is already partially filled");
    }

    const Side side = order->getSide();

    if (side == Side::BUY) {
        const PriceLevelPointer priceLevel = bids[order->getPrice()];
        priceLevel->orders.erase(priceLevel->orderIters[orderId]);
        priceLevel->orderIters.erase(orderId);
        orders.erase(orderId);

        if (priceLevel->orders.empty()) {
            bids.erase(priceLevel->price);
        }
    }

    else if (side == Side::SELL) {
        const PriceLevelPointer priceLevel = asks[order->getPrice()];
        priceLevel->orders.erase(priceLevel->orderIters[orderId]);
        priceLevel->orderIters.erase(orderId);
        orders.erase(orderId);

        if (priceLevel->orders.empty()) {
            asks.erase(priceLevel->price);
        }
    }
}

void OrderBook::matchOrders() {
    PriceLevelPointer highestBidLevel;
    PriceLevelPointer lowestAskLevel;

    if (!bids.empty() && !asks.empty()) {
        highestBidLevel = bids.begin()->second;
        lowestAskLevel = asks.begin()->second;
    }

    while (highestBidLevel && lowestAskLevel &&
           !highestBidLevel->orders.empty() && !lowestAskLevel->orders.empty() &&
           highestBidLevel->price >= lowestAskLevel->price) {
        const OrderPointer highestBidOrder = highestBidLevel->orders.front();
        const OrderPointer lowestAskOrder = lowestAskLevel->orders.front();

        // Fill orders based on remaining quantity differences
        const Quantity qty = std::min(highestBidOrder->getRemainingQuantity(), lowestAskOrder->getRemainingQuantity());
        highestBidOrder->fill(qty);
        lowestAskOrder->fill(qty);

        // Remove filled order and update pointers as needed
        if (highestBidOrder->getRemainingQuantity() == 0) {
            OrderId orderId = highestBidOrder->getOrderId();
            highestBidLevel->orders.pop_front();
            highestBidLevel->orderIters.erase(orderId);
            orders.erase(orderId);

            if (highestBidLevel->orders.empty()) {
                bids.erase(highestBidLevel->price);
                if (!bids.empty()) {
                    highestBidLevel = bids.begin()->second;
                }
                else {
                    highestBidLevel = nullptr;
                }
            }
        }
        if (lowestAskOrder->getRemainingQuantity() == 0) {
            OrderId orderId = lowestAskOrder->getOrderId();
            lowestAskLevel->orders.pop_front();
            lowestAskLevel->orderIters.erase(orderId);
            orders.erase(orderId);

            if (lowestAskLevel->orders.empty()) {
                asks.erase(lowestAskLevel->price);
                if (!asks.empty()) {
                    lowestAskLevel = asks.begin()->second;
                }
                else {
                    lowestAskLevel = nullptr;
                }
            }
        }
    }
}

OrderPointer OrderBook::getOrderById(const OrderId orderId) const {
    if (!orders.contains(orderId)) {
        throw std::logic_error("Order ('" + std::to_string(orderId) + "') does not exist");
    }

    return orders.at(orderId);
}

std::size_t OrderBook::getNumberOfOrders() const {
    return orders.size();
}


bool OrderBook::contains(const OrderId orderId) const {
    return orders.contains(orderId);;
}
#include "Order.h"

#include <stdexcept>
#include <string>

Order::Order(OrderId orderId, Side side, Price price, Quantity quantity):
    orderId{orderId}, 
    side{side}, 
    price{price}, 
    initialQuantity{quantity}, 
    remainingQuantity{quantity}, 
    status{Status::PENDING} 
{
    if (price == 0) {
        throw std::invalid_argument("Order price must be greater than zero");
    }
    if (quantity == 0) {
        throw std::invalid_argument("Order quantity must be greater than zero");
    }
}

// Getter methods
OrderId Order::getOrderId() const { return orderId; }
Side Order::getSide() const { return side; }
Price Order::getPrice() const { return price; }
Quantity Order::getInitialQuantity() const { return initialQuantity; }
Quantity Order::getRemainingQuantity() const { return remainingQuantity; }
Quantity Order::getFilledQuantity() const { return getInitialQuantity() - getRemainingQuantity(); }
Status Order::getStatus() const { return status; }

void Order::fill(const Quantity qty) {
    if (qty == 0) {
        throw std::invalid_argument("Fill quantity must be greater than zero");
    }
    if (qty > getRemainingQuantity()) {
        throw std::invalid_argument("Order ('" + std::to_string(orderId) + "') cannot be filled for more than its current remaining quantity");
    }

    remainingQuantity -= qty;
    if (remainingQuantity == 0) {
        status = Status::COMPLETELY_FILLED;
    }
    else if (remainingQuantity < getInitialQuantity()) {
        status = Status::PARTIALLY_FILLED;
    }
}

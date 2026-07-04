#include "OrderBook/OrderBook.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace {
OrderPointer order(OrderId id, Side side, Price price, Quantity quantity) {
    return std::make_shared<Order>(id, side, price, quantity);
}

void testPriceTimePriority() {
    OrderBook book;
    auto first = order(1, Side::BUY, 100, 5);
    auto second = order(2, Side::BUY, 100, 5);
    book.addOrder(first);
    book.addOrder(second);
    book.addOrder(order(3, Side::SELL, 99, 7));
    book.matchOrders();

    assert(first->getStatus() == Status::COMPLETELY_FILLED);
    assert(second->getRemainingQuantity() == 3);
    assert(book.getTrades().size() == 2);
    assert(book.getTrades()[0].buyOrderId == 1);
    assert(book.getTrades()[1].buyOrderId == 2);
}

void testBestPricesAndCancel() {
    OrderBook book;
    book.addOrder(order(1, Side::BUY, 99, 2));
    book.addOrder(order(2, Side::BUY, 100, 2));
    book.addOrder(order(3, Side::SELL, 102, 2));
    book.addOrder(order(4, Side::SELL, 101, 2));

    assert(book.getBestBid() == 100);
    assert(book.getBestAsk() == 101);
    book.cancelOrder(2);
    assert(book.getBestBid() == 99);
}

void testPartiallyFilledOrderCanBeCanceled() {
    OrderBook book;
    book.addOrder(order(1, Side::BUY, 100, 10));
    book.addOrder(order(2, Side::SELL, 100, 4));
    book.matchOrders();

    assert(book.getOrderById(1)->getRemainingQuantity() == 6);
    book.cancelOrder(1);
    assert(!book.contains(1));
    assert(book.getNumberOfOrders() == 0);
}

void testInvalidOrdersAndDuplicateIds() {
    bool rejectedZeroQuantity = false;
    try {
        (void)order(1, Side::BUY, 100, 0);
    } catch (const std::invalid_argument&) {
        rejectedZeroQuantity = true;
    }
    assert(rejectedZeroQuantity);

    OrderBook book;
    book.addOrder(order(1, Side::BUY, 100, 1));
    bool rejectedDuplicate = false;
    try {
        book.addOrder(order(1, Side::SELL, 101, 1));
    } catch (const std::logic_error&) {
        rejectedDuplicate = true;
    }
    assert(rejectedDuplicate);
}
}

int main() {
    testPriceTimePriority();
    testBestPricesAndCancel();
    testPartiallyFilledOrderCanBeCanceled();
    testInvalidOrdersAndDuplicateIds();
    std::cout << "All order book tests passed\n";
}

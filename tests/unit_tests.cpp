#include <gtest/gtest.h>
#include "book/OrderBook.h"
#include "book/AuditLog.h"
#include "book/MemoryPool.h"

TEST(OrderBookTest, BasicMatching) {
    MemoryPool<Order> pool(1024);
    AuditLog logger;
    OrderBook book(pool, logger);
    
    // Add resting Sell order
    Order sell1(1, 10050, 10, Side::SELL, OrderType::LIMIT);
    EXPECT_FALSE(book.addOrder(std::move(sell1))); // Should rest
    
    EXPECT_EQ(book.getBestAsk().value(), 10050);
    EXPECT_FALSE(book.getBestBid().has_value());

    // Add matching Buy order
    Order buy1(2, 10050, 5, Side::BUY, OrderType::LIMIT);
    EXPECT_TRUE(book.addOrder(std::move(buy1))); // Fully matches against resting ask

    EXPECT_FALSE(book.getBestBid().has_value());
    EXPECT_EQ(book.getBestAsk().value(), 10050);
}

TEST(OrderBookTest, Cancellation) {
    MemoryPool<Order> pool(1024);
    AuditLog logger;
    OrderBook book(pool, logger);
    Order buy1(1, 10000, 10, Side::BUY, OrderType::LIMIT);
    book.addOrder(std::move(buy1));
    
    EXPECT_TRUE(book.cancelOrder(1));
    EXPECT_FALSE(book.getBestBid().has_value());
    EXPECT_FALSE(book.cancelOrder(999)); // Non-existent ID
}
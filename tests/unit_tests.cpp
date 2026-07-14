#include <gtest/gtest.h>
#include "book/OrderBook.hpp"

TEST(OrderBookTest, BasicMatching) {
    OrderBook book;
    
    // Add resting Sell order
    Order sell1(1, 10050, 10, Side::SELL, OrderType::LIMIT);
    EXPECT_FALSE(book.addOrder(std::move(sell1))); // Should rest
    
    EXPECT_EQ(book.getBestAsk().value(), 10050);
    EXPECT_FALSE(book.getBestBid().has_value());

    // Add matching Buy order
    Order buy1(2, 10050, 5, Side::BUY, OrderType::LIMIT);
    EXPECT_FALSE(book.addOrder(std::move(buy1))); // Partially matches, buy1 rests
    
    EXPECT_EQ(book.getBestBid().value(), 10050);
    EXPECT_EQ(book.getBestAsk().value(), 10050);
}

TEST(OrderBookTest, Cancellation) {
    OrderBook book;
    Order buy1(1, 10000, 10, Side::BUY, OrderType::LIMIT);
    book.addOrder(std::move(buy1));
    
    EXPECT_TRUE(book.cancelOrder(1));
    EXPECT_FALSE(book.getBestBid().has_value());
    EXPECT_FALSE(book.cancelOrder(999)); // Non-existent ID
}
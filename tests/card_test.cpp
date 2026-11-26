#include <gtest/gtest.h>
#include "core/cards/card.hpp"

TEST(CardTest, DefaultConstructor) {
    Card c;
    EXPECT_EQ(c.getRank(), 1);
    EXPECT_EQ(c.getSuit(), Spades);
}

TEST(CardTest, ParameterizedConstructor) {
    Card c(14, Hearts);
    EXPECT_EQ(c.getRank(), 14);
    EXPECT_EQ(c.getSuit(), Hearts);
}

TEST(CardTest, ToStringAce) {
    Card c(14, Spades);
    EXPECT_EQ(c.toString(), "A♠");
}

TEST(CardTest, ToStringKing) {
    Card c(13, Hearts);
    EXPECT_EQ(c.toString(), "K♥");
}

TEST(CardTest, ToStringQueen) {
    Card c(12, Clubs);
    EXPECT_EQ(c.toString(), "Q♣");
}

TEST(CardTest, ToStringJack) {
    Card c(11, Diamonds);
    EXPECT_EQ(c.toString(), "J♦");
}

TEST(CardTest, ToStringNumber) {
    Card c(7, Spades);
    EXPECT_EQ(c.toString(), "7♠");
}

TEST(CardTest, ToStringTwo) {
    Card c(2, Hearts);
    EXPECT_EQ(c.toString(), "2♥");
}

TEST(CardTest, ToStringTen) {
    Card c(10, Diamonds);
    EXPECT_EQ(c.toString(), "10♦");
}

TEST(CardTest, AllSuits) {
    EXPECT_EQ(Card(14, Spades).toString(), "A♠");
    EXPECT_EQ(Card(14, Hearts).toString(), "A♥");
    EXPECT_EQ(Card(14, Clubs).toString(), "A♣");
    EXPECT_EQ(Card(14, Diamonds).toString(), "A♦");
}

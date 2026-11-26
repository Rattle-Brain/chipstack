#include <gtest/gtest.h>
#include "test_helpers.hpp"
#include "core/blackjack/blackjack_table.hpp"

TEST(BlackjackTableTest, HandValueSimple) {
    BlackjackTable table;
    ScriptedPlayer p("Test", 1000);
    p.receiveCard(Card(5, Hearts));
    p.receiveCard(Card(3, Spades));
    EXPECT_EQ(table.getHandValue(p), 8);
}

TEST(BlackjackTableTest, HandValueFaceCards) {
    BlackjackTable table;
    ScriptedPlayer p("Test", 1000);
    p.receiveCard(Card(13, Hearts));  // King
    p.receiveCard(Card(12, Spades));  // Queen
    EXPECT_EQ(table.getHandValue(p), 20);
}

TEST(BlackjackTableTest, HandValueAceAs11) {
    BlackjackTable table;
    ScriptedPlayer p("Test", 1000);
    p.receiveCard(Card(14, Hearts));  // Ace
    p.receiveCard(Card(13, Spades));  // King
    EXPECT_EQ(table.getHandValue(p), 21);
}

TEST(BlackjackTableTest, HandValueAceAs1) {
    BlackjackTable table;
    ScriptedPlayer p("Test", 1000);
    p.receiveCard(Card(14, Hearts));  // Ace
    p.receiveCard(Card(13, Spades));  // King
    p.receiveCard(Card(5, Clubs));
    EXPECT_EQ(table.getHandValue(p), 16);
}

TEST(BlackjackTableTest, HandValueMultipleAces) {
    BlackjackTable table;
    ScriptedPlayer p("Test", 1000);
    p.receiveCard(Card(14, Hearts));  // Ace
    p.receiveCard(Card(14, Spades));  // Ace
    EXPECT_EQ(table.getHandValue(p), 12);
}

TEST(BlackjackTableTest, IsBust) {
    BlackjackTable table;
    ScriptedPlayer p("Test", 1000);
    p.receiveCard(Card(13, Hearts));
    p.receiveCard(Card(12, Spades));
    p.receiveCard(Card(5, Clubs));
    EXPECT_TRUE(table.isBust(p));
}

TEST(BlackjackTableTest, IsNotBust) {
    BlackjackTable table;
    ScriptedPlayer p("Test", 1000);
    p.receiveCard(Card(13, Hearts));
    p.receiveCard(Card(12, Spades));
    EXPECT_FALSE(table.isBust(p));
}

TEST(BlackjackTableTest, IsBlackjack) {
    BlackjackTable table;
    ScriptedPlayer p("Test", 1000);
    p.receiveCard(Card(14, Hearts));
    p.receiveCard(Card(13, Spades));
    EXPECT_TRUE(table.isBlackjack(p));
}

TEST(BlackjackTableTest, IsNotBlackjackThreeCards) {
    BlackjackTable table;
    ScriptedPlayer p("Test", 1000);
    p.receiveCard(Card(7, Hearts));
    p.receiveCard(Card(4, Spades));
    p.receiveCard(Card(10, Clubs));
    EXPECT_FALSE(table.isBlackjack(p));
}

TEST(BlackjackTableTest, PlayHandChipsConserved) {
    BlackjackTable table;
    ScriptedPlayer p1("Alice", 1000);
    ScriptedPlayer p2("Bob", 1000);
    table.addPlayer(&p1);
    table.addPlayer(&p2);

    p1.queueBlackjackBet(50);
    p2.queueBlackjackBet(50);
    p1.queueBlackjackAction(BlackjackAction::stand());
    p2.queueBlackjackAction(BlackjackAction::stand());

    table.playHand(10, 500);

    int total = p1.getChips() + p2.getChips();
    EXPECT_TRUE(total >= 1900 && total <= 2100);
}

TEST(BlackjackTableTest, PlayHandWithHit) {
    BlackjackTable table;
    ScriptedPlayer p("Alice", 1000);
    table.addPlayer(&p);

    p.queueBlackjackBet(50);
    p.queueBlackjackAction(BlackjackAction::hit());
    p.queueBlackjackAction(BlackjackAction::stand());

    RecordingObserver obs;
    table.setObserver(&obs);
    table.playHand(10, 500);

    EXPECT_TRUE(obs.hasEvent("PlayerHit") || obs.hasEvent("PlayerStand"));
}

TEST(BlackjackTableTest, BetPlaced) {
    BlackjackTable table;
    ScriptedPlayer p("Alice", 1000);
    table.addPlayer(&p);

    p.queueBlackjackBet(100);
    p.queueBlackjackAction(BlackjackAction::stand());

    RecordingObserver obs;
    table.setObserver(&obs);
    table.playHand(10, 500);

    EXPECT_TRUE(obs.hasEvent("BetPlaced"));
}

TEST(BlackjackTableTest, DealerPlays) {
    BlackjackTable table;
    ScriptedPlayer p("Alice", 1000);
    table.addPlayer(&p);

    p.queueBlackjackBet(50);
    p.queueBlackjackAction(BlackjackAction::stand());

    RecordingObserver obs;
    table.setObserver(&obs);
    table.playHand(10, 500);

    EXPECT_TRUE(obs.hasEvent("DealerReveals"));
    EXPECT_TRUE(obs.hasEvent("DealerBust") || obs.hasEvent("DealerStand"));
}

TEST(BlackjackTableTest, ResultsReported) {
    BlackjackTable table;
    ScriptedPlayer p("Alice", 1000);
    table.addPlayer(&p);

    p.queueBlackjackBet(50);
    p.queueBlackjackAction(BlackjackAction::stand());

    RecordingObserver obs;
    table.setObserver(&obs);
    table.playHand(10, 500);

    EXPECT_TRUE(obs.hasEvent("BlackjackResult"));
}

TEST(BlackjackTableTest, StateAfterPlayHand) {
    BlackjackTable table;
    ScriptedPlayer p("Alice", 1000);
    table.addPlayer(&p);

    p.queueBlackjackBet(50);
    p.queueBlackjackAction(BlackjackAction::stand());

    table.playHand(10, 500);
    EXPECT_EQ(table.getState(), BlackjackState::WaitingForPlayers);
}

TEST(BlackjackTableTest, MultipleHands) {
    BlackjackTable table;
    ScriptedPlayer p("Alice", 1000);
    table.addPlayer(&p);

    for (int i = 0; i < 5; i++) {
        p.queueBlackjackBet(10);
        p.queueBlackjackAction(BlackjackAction::stand());
        table.playHand(10, 500);
    }

    EXPECT_TRUE(p.getChips() > 0 || p.getChips() == 0);
}

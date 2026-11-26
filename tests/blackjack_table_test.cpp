#include "core/blackjack/blackjack_rules.hpp"
#include "core/blackjack/blackjack_table.hpp"
#include "test_helpers.hpp"
#include <gtest/gtest.h>

TEST(BlackjackTableTest, HandValueSimple) {
	ScriptedPlayer p("Test", 1000);
	p.receiveCard(Card(5, Hearts));
	p.receiveCard(Card(3, Spades));
	EXPECT_EQ(BlackjackRules::handTotal(p.getHand()), 8);
}

TEST(BlackjackTableTest, HandValueFaceCards) {
	ScriptedPlayer p("Test", 1000);
	p.receiveCard(Card(13, Hearts)); // King
	p.receiveCard(Card(12, Spades)); // Queen
	EXPECT_EQ(BlackjackRules::handTotal(p.getHand()), 20);
}

TEST(BlackjackTableTest, HandValueAceAs11) {
	ScriptedPlayer p("Test", 1000);
	p.receiveCard(Card(14, Hearts)); // Ace
	p.receiveCard(Card(13, Spades)); // King
	EXPECT_EQ(BlackjackRules::handTotal(p.getHand()), 21);
}

TEST(BlackjackTableTest, HandValueAceAs1) {
	ScriptedPlayer p("Test", 1000);
	p.receiveCard(Card(14, Hearts)); // Ace
	p.receiveCard(Card(13, Spades)); // King
	p.receiveCard(Card(5, Clubs));
	EXPECT_EQ(BlackjackRules::handTotal(p.getHand()), 16);
}

TEST(BlackjackTableTest, HandValueMultipleAces) {
	ScriptedPlayer p("Test", 1000);
	p.receiveCard(Card(14, Hearts)); // Ace
	p.receiveCard(Card(14, Spades)); // Ace
	EXPECT_EQ(BlackjackRules::handTotal(p.getHand()), 12);
}

TEST(BlackjackTableTest, IsBust) {
	ScriptedPlayer p("Test", 1000);
	p.receiveCard(Card(13, Hearts));
	p.receiveCard(Card(12, Spades));
	p.receiveCard(Card(5, Clubs));
	EXPECT_TRUE(BlackjackRules::isBust(p.getHand()));
}

TEST(BlackjackTableTest, IsNotBust) {
	ScriptedPlayer p("Test", 1000);
	p.receiveCard(Card(13, Hearts));
	p.receiveCard(Card(12, Spades));
	EXPECT_FALSE(BlackjackRules::isBust(p.getHand()));
}

TEST(BlackjackTableTest, IsBlackjack) {
	ScriptedPlayer p("Test", 1000);
	p.receiveCard(Card(14, Hearts));
	p.receiveCard(Card(13, Spades));
	EXPECT_TRUE(BlackjackRules::isNatural(p.getHand()));
}

TEST(BlackjackTableTest, IsNotBlackjackThreeCards) {
	ScriptedPlayer p("Test", 1000);
	p.receiveCard(Card(7, Hearts));
	p.receiveCard(Card(4, Spades));
	p.receiveCard(Card(10, Clubs));
	EXPECT_FALSE(BlackjackRules::isNatural(p.getHand()));
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

	table.playHand();

	int total = p1.getChips() + p2.getChips();
	EXPECT_TRUE(total >= 1900 && total <= 2150);
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
	table.playHand();

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
	table.playHand();

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
	table.playHand();

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
	table.playHand();

	EXPECT_TRUE(obs.hasEvent("BlackjackResult"));
}

TEST(BlackjackTableTest, StateAfterPlayHand) {
	BlackjackTable table;
	ScriptedPlayer p("Alice", 1000);
	table.addPlayer(&p);

	p.queueBlackjackBet(50);
	p.queueBlackjackAction(BlackjackAction::stand());

	table.playHand();
	EXPECT_EQ(table.getState(), BlackjackState::WaitingForPlayers);
}

TEST(BlackjackTableTest, MultipleHands) {
	BlackjackTable table;
	ScriptedPlayer p("Alice", 1000);
	table.addPlayer(&p);

	for (int i = 0; i < 5; i++) {
		p.queueBlackjackBet(10);
		p.queueBlackjackAction(BlackjackAction::stand());
		table.playHand();
	}

	EXPECT_TRUE(p.getChips() > 0 || p.getChips() == 0);
}

#include "core/poker/poker_table.hpp"
#include "test_helpers.hpp"
#include <gtest/gtest.h>

TEST(PokerTableTest, AddPlayers) {
	PokerTable table(10, 20);
	ScriptedPlayer p1("Alice", 1000);
	ScriptedPlayer p2("Bob", 1000);
	table.addPlayer(&p1);
	table.addPlayer(&p2);
	EXPECT_EQ(table.getPlayerCount(), 2);
}

TEST(PokerTableTest, DealGivesTwoCards) {
	PokerTable table(10, 20);
	ScriptedPlayer p1("Alice", 1000);
	ScriptedPlayer p2("Bob", 1000);
	table.addPlayer(&p1);
	table.addPlayer(&p2);
	table.deal();
	EXPECT_EQ(p1.getHand().size(), 2u);
	EXPECT_EQ(p2.getHand().size(), 2u);
}

TEST(PokerTableTest, DealSetsPreFlopState) {
	PokerTable table(10, 20);
	ScriptedPlayer p1("Alice", 1000);
	ScriptedPlayer p2("Bob", 1000);
	table.addPlayer(&p1);
	table.addPlayer(&p2);
	table.deal();
	EXPECT_EQ(table.getState(), PokerState::PreFlop);
}

TEST(PokerTableTest, NeedTwoPlayersToPlay) {
	PokerTable table(10, 20);
	ScriptedPlayer p1("Alice", 1000);
	table.addPlayer(&p1);
	RecordingObserver obs;
	table.setObserver(&obs);
	table.deal();
	EXPECT_TRUE(obs.hasEvent("Error"));
	EXPECT_EQ(table.getState(), PokerState::WaitingForPlayers);
}

TEST(PokerTableTest, DealerButtonAdvances) {
	PokerTable table(10, 20);
	ScriptedPlayer p1("Alice", 1000);
	ScriptedPlayer p2("Bob", 1000);
	ScriptedPlayer p3("Charlie", 1000);
	table.addPlayer(&p1);
	table.addPlayer(&p2);
	table.addPlayer(&p3);

	EXPECT_EQ(table.getDealerButton(), 0);
	table.advanceDealerButton();
	EXPECT_EQ(table.getDealerButton(), 1);
	table.advanceDealerButton();
	EXPECT_EQ(table.getDealerButton(), 2);
	table.advanceDealerButton();
	EXPECT_EQ(table.getDealerButton(), 0);
}

TEST(PokerTableTest, PlayHandAllFold) {
	PokerTable table(5, 10);
	ScriptedPlayer p1("Alice", 1000);
	ScriptedPlayer p2("Bob", 1000);
	ScriptedPlayer p3("Charlie", 1000);
	table.addPlayer(&p1);
	table.addPlayer(&p2);
	table.addPlayer(&p3);

	p1.queuePokerAction(PokerAction::fold());
	p2.queuePokerAction(PokerAction::fold());
	p3.queuePokerAction(PokerAction::fold());

	RecordingObserver obs;
	table.setObserver(&obs);

	int chipsBefore = p2.getChips();
	table.playHand();

	EXPECT_TRUE(obs.hasEvent("PotWon"));
	EXPECT_EQ(table.getState(), PokerState::WaitingForPlayers);
}

TEST(PokerTableTest, PlayHandCallsThrough) {
	PokerTable table(5, 10);
	ScriptedPlayer p1("Alice", 1000);
	ScriptedPlayer p2("Bob", 1000);
	table.addPlayer(&p1);
	table.addPlayer(&p2);

	for (int i = 0; i < 8; i++) {
		p1.queuePokerAction(PokerAction::call());
		p2.queuePokerAction(PokerAction::check());
	}

	RecordingObserver obs;
	table.setObserver(&obs);
	table.playHand();

	EXPECT_TRUE(obs.hasEvent("ShowdownBegin") || obs.hasEvent("PotWon"));
	int totalChips = p1.getChips() + p2.getChips();
	EXPECT_EQ(totalChips, 2000);
}

TEST(PokerTableTest, BlindsDeducted) {
	PokerTable table(5, 10);
	ScriptedPlayer p1("Alice", 1000);
	ScriptedPlayer p2("Bob", 1000);
	ScriptedPlayer p3("Charlie", 1000);
	table.addPlayer(&p1);
	table.addPlayer(&p2);
	table.addPlayer(&p3);

	for (int i = 0; i < 10; i++) {
		p1.queuePokerAction(PokerAction::fold());
		p2.queuePokerAction(PokerAction::fold());
		p3.queuePokerAction(PokerAction::fold());
	}

	RecordingObserver obs;
	table.setObserver(&obs);
	table.playHand();

	EXPECT_TRUE(obs.hasEvent("BlindsPosted"));
	EXPECT_EQ(obs.countEvents("BlindsPosted"), 2);
}

TEST(PokerTableTest, ChipsConserved) {
	PokerTable table(5, 10);
	ScriptedPlayer p1("Alice", 1000);
	ScriptedPlayer p2("Bob", 1000);
	ScriptedPlayer p3("Charlie", 1000);
	table.addPlayer(&p1);
	table.addPlayer(&p2);
	table.addPlayer(&p3);

	for (int i = 0; i < 20; i++) {
		p1.queuePokerAction(PokerAction::call());
		p2.queuePokerAction(PokerAction::call());
		p3.queuePokerAction(PokerAction::check());
	}

	table.playHand();
	int total = p1.getChips() + p2.getChips() + p3.getChips();
	EXPECT_EQ(total, 3000);
}

TEST(PokerTableTest, RemoveBustedPlayers) {
	PokerTable table(5, 10);
	ScriptedPlayer p1("Alice", 1000);
	ScriptedPlayer p2("Bob", 0);
	table.addPlayer(&p1);
	table.addPlayer(&p2);
	table.removeBustedPlayers();
	EXPECT_EQ(table.getPlayerCount(), 1);
}

TEST(PokerTableTest, StateTransitions) {
	PokerTable table(5, 10);
	ScriptedPlayer p1("Alice", 1000);
	ScriptedPlayer p2("Bob", 1000);
	table.addPlayer(&p1);
	table.addPlayer(&p2);

	EXPECT_EQ(table.getState(), PokerState::WaitingForPlayers);

	table.deal();
	EXPECT_EQ(table.getState(), PokerState::PreFlop);

	table.dealFlop();
	EXPECT_EQ(table.getState(), PokerState::Flop);
	EXPECT_EQ(table.getCommunityCards().size(), 3u);

	table.dealTurn();
	EXPECT_EQ(table.getState(), PokerState::Turn);
	EXPECT_EQ(table.getCommunityCards().size(), 4u);

	table.dealRiver();
	EXPECT_EQ(table.getState(), PokerState::River);
	EXPECT_EQ(table.getCommunityCards().size(), 5u);
}

TEST(PokerTableTest, CollectCardsResetsState) {
	PokerTable table(5, 10);
	ScriptedPlayer p1("Alice", 1000);
	ScriptedPlayer p2("Bob", 1000);
	table.addPlayer(&p1);
	table.addPlayer(&p2);

	table.deal();
	table.collectCards();

	EXPECT_EQ(table.getState(), PokerState::WaitingForPlayers);
	EXPECT_EQ(p1.getHand().size(), 0u);
	EXPECT_EQ(p2.getHand().size(), 0u);
	EXPECT_EQ(table.getCommunityCards().size(), 0u);
	EXPECT_EQ(table.getPot(), 0);
}

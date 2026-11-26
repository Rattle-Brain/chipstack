#include "core/poker/betting_round.hpp"
#include "test_helpers.hpp"
#include <gtest/gtest.h>

static PokerSeats makeSeats(std::vector<ScriptedPlayer*>& players) {
	PokerSeats seats;
	for (auto* p : players) {
		seats.push_back({p});
	}
	return seats;
}

TEST(BettingRoundTest, CheckAction) {
	ScriptedPlayer p1("Alice", 1000);
	ScriptedPlayer p2("Bob", 1000);
	std::vector<ScriptedPlayer*> ptrs = {&p1, &p2};
	auto seats = makeSeats(ptrs);

	p1.queuePokerAction(PokerAction::check());
	p2.queuePokerAction(PokerAction::check());

	BettingRound round(seats, 0, {5, 10});
	int pot = round.execute(BettingRoundType::Flop, {}, 0, 1);

	EXPECT_EQ(pot, 0);
	EXPECT_EQ(p1.getChips(), 1000);
	EXPECT_EQ(p2.getChips(), 1000);
}

TEST(BettingRoundTest, CallAction) {
	ScriptedPlayer p1("Alice", 1000);
	ScriptedPlayer p2("Bob", 1000);
	ScriptedPlayer p3("Charlie", 1000);
	std::vector<ScriptedPlayer*> ptrs = {&p1, &p2, &p3};
	auto seats = makeSeats(ptrs);

	p1.queuePokerAction(PokerAction::call());
	p2.queuePokerAction(PokerAction::call());
	p3.queuePokerAction(PokerAction::check());

	BettingRound round(seats, 0, {5, 10});
	int pot = round.execute(BettingRoundType::PreFlop, {}, 0, 0);

	EXPECT_GT(pot, 0);
}

TEST(BettingRoundTest, FoldAction) {
	ScriptedPlayer p1("Alice", 1000);
	ScriptedPlayer p2("Bob", 1000);
	std::vector<ScriptedPlayer*> ptrs = {&p1, &p2};
	auto seats = makeSeats(ptrs);

	p1.queuePokerAction(PokerAction::fold());
	p2.queuePokerAction(PokerAction::check());

	BettingRound round(seats, 0, {5, 10});
	round.execute(BettingRoundType::Flop, {}, 0, 1);

	EXPECT_EQ(seats[0].state, SeatState::Folded);
	EXPECT_EQ(seats[1].state, SeatState::Active);
}

TEST(BettingRoundTest, RaiseAction) {
	ScriptedPlayer p1("Alice", 1000);
	ScriptedPlayer p2("Bob", 1000);
	std::vector<ScriptedPlayer*> ptrs = {&p1, &p2};
	auto seats = makeSeats(ptrs);

	// p1 acts first (startIndex=0), raises to 40, then p2 calls
	p1.queuePokerAction(PokerAction::raise(40));
	p2.queuePokerAction(PokerAction::call());

	BettingRound round(seats, 0, {5, 10});
	int pot = round.execute(BettingRoundType::Flop, {}, 0, 0);

	EXPECT_EQ(pot, 80);
	EXPECT_EQ(p1.getChips(), 960);
	EXPECT_EQ(p2.getChips(), 960);
}

TEST(BettingRoundTest, AllInAction) {
	ScriptedPlayer p1("Alice", 100);
	ScriptedPlayer p2("Bob", 1000);
	ScriptedPlayer p3("Charlie", 1000);
	std::vector<ScriptedPlayer*> ptrs = {&p1, &p2, &p3};
	auto seats = makeSeats(ptrs);

	// p1 goes all-in, p2 calls, p3 folds
	p1.queuePokerAction(PokerAction::allIn());
	p2.queuePokerAction(PokerAction::call());
	p3.queuePokerAction(PokerAction::fold());

	BettingRound round(seats, 0, {5, 10});
	int pot = round.execute(BettingRoundType::Flop, {}, 0, 0);

	EXPECT_EQ(p1.getChips(), 0);
	EXPECT_EQ(seats[0].state, SeatState::AllIn);
	EXPECT_EQ(pot, 200);
}

TEST(BettingRoundTest, ValidationCheckWhenMustCallBecomesFold) {
	ScriptedPlayer p1("Alice", 1000);
	ScriptedPlayer p2("Bob", 1000);
	std::vector<ScriptedPlayer*> ptrs = {&p1, &p2};
	auto seats = makeSeats(ptrs);

	p1.queuePokerAction(PokerAction::raise(40));
	p2.queuePokerAction(PokerAction::check());

	RecordingObserver obs;
	BettingRound round(seats, 0, {5, 10}, &obs);
	round.execute(BettingRoundType::Flop, {}, 0, 1);

	EXPECT_EQ(seats[1].state, SeatState::Folded);
}

TEST(BettingRoundTest, CountActivePlayers) {
	ScriptedPlayer p1("Alice", 1000);
	ScriptedPlayer p2("Bob", 1000);
	ScriptedPlayer p3("Charlie", 1000);
	std::vector<ScriptedPlayer*> ptrs = {&p1, &p2, &p3};
	auto seats = makeSeats(ptrs);
	seats[1].state = SeatState::Folded;
	EXPECT_EQ(countActiveSeats(seats), 2);
	EXPECT_EQ(countSeatsInHand(seats), 2);
}

TEST(BettingRoundTest, ObserverReceivesEvents) {
	ScriptedPlayer p1("Alice", 1000);
	ScriptedPlayer p2("Bob", 1000);
	std::vector<ScriptedPlayer*> ptrs = {&p1, &p2};
	auto seats = makeSeats(ptrs);

	p1.queuePokerAction(PokerAction::check());
	p2.queuePokerAction(PokerAction::check());

	RecordingObserver obs;
	BettingRound round(seats, 0, {5, 10}, &obs);
	round.execute(BettingRoundType::Flop, {}, 0, 1);

	EXPECT_TRUE(obs.hasEvent("PlayerAction"));
	EXPECT_EQ(obs.countEvents("PlayerAction"), 2);
}

TEST(BettingRoundTest, PreFlopBlinds) {
	ScriptedPlayer p1("Alice", 1000);
	ScriptedPlayer p2("Bob", 1000);
	ScriptedPlayer p3("Charlie", 1000);
	std::vector<ScriptedPlayer*> ptrs = {&p1, &p2, &p3};
	auto seats = makeSeats(ptrs);

	p1.queuePokerAction(PokerAction::fold());
	p2.queuePokerAction(PokerAction::fold());
	p3.queuePokerAction(PokerAction::fold());

	RecordingObserver obs;
	BettingRound round(seats, 0, {10, 20}, &obs);
	int pot = round.execute(BettingRoundType::PreFlop, {}, 0, 3);

	EXPECT_TRUE(obs.hasEvent("BlindsPosted"));
	EXPECT_EQ(obs.countEvents("BlindsPosted"), 2);
	EXPECT_GT(pot, 0);
}

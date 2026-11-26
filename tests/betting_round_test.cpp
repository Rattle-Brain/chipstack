#include <gtest/gtest.h>
#include "test_helpers.hpp"
#include "core/poker/betting_round.hpp"

static std::vector<PlayerSeat> makeSeats(std::vector<ScriptedPlayer*>& players) {
    std::vector<PlayerSeat> seats;
    for (auto* p : players) {
        PlayerSeat seat;
        seat.player = p;
        seat.state = PlayerHandState::Active;
        seat.currentBet = 0;
        seat.totalInvested = 0;
        seats.push_back(seat);
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

    BettingRound round(seats, 0, 10);
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

    BettingRound round(seats, 0, 10);
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

    BettingRound round(seats, 0, 10);
    round.execute(BettingRoundType::Flop, {}, 0, 1);

    EXPECT_EQ(seats[0].state, PlayerHandState::Folded);
    EXPECT_EQ(seats[1].state, PlayerHandState::Active);
}

TEST(BettingRoundTest, RaiseAction) {
    ScriptedPlayer p1("Alice", 1000);
    ScriptedPlayer p2("Bob", 1000);
    std::vector<ScriptedPlayer*> ptrs = {&p1, &p2};
    auto seats = makeSeats(ptrs);

    // p1 acts first (startIndex=0), raises to 40, then p2 calls
    p1.queuePokerAction(PokerAction::raise(40));
    p2.queuePokerAction(PokerAction::call());

    BettingRound round(seats, 0, 10);
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

    BettingRound round(seats, 0, 10);
    int pot = round.execute(BettingRoundType::Flop, {}, 0, 0);

    EXPECT_EQ(p1.getChips(), 0);
    EXPECT_EQ(seats[0].state, PlayerHandState::AllIn);
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
    BettingRound round(seats, 0, 10, &obs);
    round.execute(BettingRoundType::Flop, {}, 0, 1);

    EXPECT_EQ(seats[1].state, PlayerHandState::Folded);
}

TEST(BettingRoundTest, CountActivePlayers) {
    ScriptedPlayer p1("Alice", 1000);
    ScriptedPlayer p2("Bob", 1000);
    ScriptedPlayer p3("Charlie", 1000);
    std::vector<ScriptedPlayer*> ptrs = {&p1, &p2, &p3};
    auto seats = makeSeats(ptrs);
    seats[1].state = PlayerHandState::Folded;

    BettingRound round(seats, 0, 10);
    EXPECT_EQ(round.countActivePlayers(), 2);
    EXPECT_EQ(round.countNonFoldedPlayers(), 2);
}

TEST(BettingRoundTest, ObserverReceivesEvents) {
    ScriptedPlayer p1("Alice", 1000);
    ScriptedPlayer p2("Bob", 1000);
    std::vector<ScriptedPlayer*> ptrs = {&p1, &p2};
    auto seats = makeSeats(ptrs);

    p1.queuePokerAction(PokerAction::check());
    p2.queuePokerAction(PokerAction::check());

    RecordingObserver obs;
    BettingRound round(seats, 0, 10, &obs);
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
    BettingRound round(seats, 0, 20, &obs);
    int pot = round.execute(BettingRoundType::PreFlop, {}, 0, 3);

    EXPECT_TRUE(obs.hasEvent("BlindsPosted"));
    EXPECT_EQ(obs.countEvents("BlindsPosted"), 2);
    EXPECT_GT(pot, 0);
}

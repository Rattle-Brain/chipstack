#ifndef POKER_TABLE_HPP
#define POKER_TABLE_HPP

#include "core/table/itable.hpp"
#include "betting_round.hpp"
#include "core/poker/hand_evaluator.hpp"

enum class PokerState {
    WaitingForPlayers,
    PreFlop,
    Flop,
    Turn,
    River,
    Showdown,
    HandComplete
};

class PokerTable : public ITable {
  public:
    PokerTable(int smallBlind = 10, int bigBlind = 20);
    ~PokerTable();

    void deal() override;
    void collectCards() override;

    void dealFlop();
    void dealTurn();
    void dealRiver();
    void showdown();

    void playHand();

    const std::vector<Card>& getCommunityCards() const;
    std::string showCommunityCards();

    int getDealerButton() const;
    void advanceDealerButton();

    int getPot() const;
    void addToPot(int amount);
    void resetPot();

    PokerState getState() const;
    const char* getStateName() const;

    int getSmallBlind() const;
    int getBigBlind() const;

    const std::vector<PlayerSeat>& getSeats() const;

  private:
    std::vector<Card> communityCards;
    std::vector<PlayerSeat> seats;
    int dealerButton;
    int pot;
    int smallBlind;
    int bigBlind;
    PokerState state;

    void initSeats();
    void runBettingRound(BettingRoundType roundType);
    int getBettingStartIndex(BettingRoundType roundType) const;
    void awardPotToLastPlayer();
};

#endif

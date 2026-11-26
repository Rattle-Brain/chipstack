#ifndef BLACKJACK_TABLE_HPP
#define BLACKJACK_TABLE_HPP

#include "core/table/itable.hpp"
#include "blackjack_action.hpp"
#include <map>

enum class BlackjackState {
    WaitingForPlayers,
    PlayerTurns,
    CrupierTurn,
    Resolving,
    HandComplete
};

class BlackjackTable : public ITable {
  public:
    BlackjackTable();
    ~BlackjackTable();

    void deal() override;
    void collectCards() override;

    void playHand(int minBet = 10, int maxBet = 500);

    bool placeBet(IPlayer& player, int amount);
    void hitPlayer(IPlayer& player);
    void standPlayer();
    void playCrupierTurn();

    int getHandValue(const IPlayer& player) const;
    bool isBust(const IPlayer& player) const;
    bool isBlackjack(const IPlayer& player) const;

    BlackjackState getState() const;
    const char* getStateName() const;
    IPlayer* getCurrentPlayer() const;

    const std::map<IPlayer*, int>& getBets() const;
    const Crupier& getCrupier() const;
    int getCurrentPlayerIndex() const;

  private:
    BlackjackState state;
    int currentPlayerIndex;
    std::map<IPlayer*, int> bets;

    void advancePlayer();
    void resolve();
    bool hasSoftAce(const IPlayer& player) const;
    int getDealerUpcard() const;
    BlackjackContext buildContext(IPlayer& player) const;
    static std::string showCards(const IPlayer& player);
};

#endif

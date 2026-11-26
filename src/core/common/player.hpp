#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "iplayer.hpp"
#include "decision_delegate.hpp"
#include "core/poker/betting_context.hpp"

class Player : public IPlayer {
  public:
    Player(std::string playerName, int initialChips);
    ~Player();

    void setDelegate(IDecisionDelegate* delegate) { delegate_ = delegate; }

    PokerAction decideAction(const BettingContext& context) override;
    BlackjackAction decideBlackjackPlay(const BlackjackContext& context) override;
    int decideBlackjackBet(const BlackjackBetContext& context) override;

  private:
    IDecisionDelegate* delegate_ = nullptr;
};

#endif

#ifndef DECISION_DELEGATE_HPP
#define DECISION_DELEGATE_HPP

#include "core/poker/poker_action.hpp"
#include "core/poker/betting_context.hpp"
#include "core/blackjack/blackjack_action.hpp"

class IDecisionDelegate {
  public:
    virtual ~IDecisionDelegate() = default;
    virtual PokerAction getPokerAction(const BettingContext& context) = 0;
    virtual BlackjackAction getBlackjackAction(const BlackjackContext& context) = 0;
    virtual int getBlackjackBet(const BlackjackBetContext& context) = 0;
};

#endif

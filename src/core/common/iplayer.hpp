#ifndef IPLAYER_HPP
#define IPLAYER_HPP

#include <string>
#include <vector>
#include "core/cards/card.hpp"
#include "core/poker/poker_action.hpp"
#include "core/blackjack/blackjack_action.hpp"

struct BettingContext;

class IPlayer {
  public:
    virtual ~IPlayer() {}

    void receiveCard(Card card) {
      hand.push_back(card);
    }

    const std::vector<Card>& getHand() const {
      return hand;
    }

    void clearHand() {
      hand.clear();
    }

    std::string showHand() {
      std::string result = name + ": ";
      for (size_t i = 0; i < hand.size(); i++) {
        result += hand[i].toString();
        if (i < hand.size() - 1) result += " ";
      }
      return result;
    }

    std::string getName() const {
      return name;
    }

    int getChips() const {
      return chips;
    }

    bool placeBet(int amount) {
      if (amount > chips) return false;
      chips -= amount;
      return true;
    }

    void receiveChips(int amount) {
      chips += amount;
    }

    bool isBustedOut() const {
      return chips <= 0;
    }

    virtual PokerAction decideAction(const BettingContext& /*context*/) {
      return PokerAction::fold();
    }

    virtual BlackjackAction decideBlackjackPlay(const BlackjackContext& /*context*/) {
      return BlackjackAction::stand();
    }

    virtual int decideBlackjackBet(const BlackjackBetContext& context) {
      return context.minBet;
    }

  protected:
    std::string name;
    std::vector<Card> hand;
    int chips = 0;
};

#endif

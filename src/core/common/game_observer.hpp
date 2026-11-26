#ifndef GAME_OBSERVER_HPP
#define GAME_OBSERVER_HPP

#include <string>
#include <vector>

class IGameObserver {
  public:
    virtual ~IGameObserver() = default;

    // Poker events
    virtual void onHandStarted(const std::string& dealer, int sb, int bb) {}
    virtual void onPlayerCards(const std::string& player,
                               const std::string& card1, const std::string& card2) {}
    virtual void onBlindsPosted(const std::string& player, int amount,
                                 const std::string& blindType, int pot,
                                 int playerChips) {}
    virtual void onPhaseChanged(const std::string& phase,
                                 const std::string& communityCards) {}
    virtual void onPlayerAction(const std::string& player,
                                 const std::string& action, int amount,
                                 int pot, int playerChips) {}
    virtual void onPotWon(const std::string& winner, int amount,
                           const std::string& handRank) {}
    virtual void onSplitPot(const std::vector<std::string>& winners,
                             int shareEach, const std::string& handRank) {}
    virtual void onShowdownReveal(const std::string& player,
                                   const std::string& cards,
                                   const std::string& handRank) {}
    virtual void onShowdownBegin(const std::string& communityCards) {}

    // Blackjack events
    virtual void onBetPlaced(const std::string& player, int amount) {}
    virtual void onDealingStarted() {}
    virtual void onPlayerDealt(const std::string& player,
                                const std::string& cards, int value) {}
    virtual void onDealerDealt(const std::string& visibleCard) {}
    virtual void onPlayerHit(const std::string& player,
                              const std::string& cards, int value) {}
    virtual void onPlayerStand(const std::string& player, int value) {}
    virtual void onPlayerBust(const std::string& player) {}
    virtual void onPlayerDoubleDown(const std::string& player, int newBet) {}
    virtual void onDealerReveals(const std::string& cards, int value) {}
    virtual void onDealerHits(const std::string& cards, int value) {}
    virtual void onDealerBust() {}
    virtual void onDealerStand(int value) {}
    virtual void onBlackjackResult(const std::string& player,
                                    const std::string& outcome, int chips) {}

    // Common
    virtual void onPlayerBustedOut(const std::string& player) {}
    virtual void onError(const std::string& message) {}
};

#endif

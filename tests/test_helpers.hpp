#ifndef TEST_HELPERS_HPP
#define TEST_HELPERS_HPP

#include "core/common/iplayer.hpp"
#include "core/common/game_observer.hpp"
#include "core/poker/poker_action.hpp"
#include "core/poker/betting_context.hpp"
#include "core/blackjack/blackjack_action.hpp"
#include <queue>
#include <string>
#include <vector>

class ScriptedPlayer : public IPlayer {
  public:
    ScriptedPlayer(const std::string& playerName, int initialChips) {
        name = playerName;
        chips = initialChips;
    }

    void queuePokerAction(PokerAction a) { pokerActions.push(a); }
    void queueBlackjackAction(BlackjackAction a) { bjActions.push(a); }
    void queueBlackjackBet(int bet) { bjBets.push(bet); }

    PokerAction decideAction(const BettingContext&) override {
        if (pokerActions.empty()) return PokerAction::fold();
        PokerAction a = pokerActions.front();
        pokerActions.pop();
        return a;
    }

    BlackjackAction decideBlackjackPlay(const BlackjackContext&) override {
        if (bjActions.empty()) return BlackjackAction::stand();
        BlackjackAction a = bjActions.front();
        bjActions.pop();
        return a;
    }

    int decideBlackjackBet(const BlackjackBetContext& context) override {
        if (bjBets.empty()) return context.minBet;
        int bet = bjBets.front();
        bjBets.pop();
        return bet;
    }

  private:
    std::queue<PokerAction> pokerActions;
    std::queue<BlackjackAction> bjActions;
    std::queue<int> bjBets;
};

struct RecordedEvent {
    std::string type;
    std::string detail;
    int amount = 0;
};

class RecordingObserver : public IGameObserver {
  public:
    std::vector<RecordedEvent> events;

    void onHandStarted(const std::string& dealer, int sb, int bb) override {
        events.push_back({"HandStarted", dealer, sb});
    }
    void onPlayerCards(const std::string& player, const std::string& c1, const std::string& c2) override {
        events.push_back({"PlayerCards", player + ": " + c1 + " " + c2, 0});
    }
    void onBlindsPosted(const std::string& player, int amount, const std::string& blindType, int, int) override {
        events.push_back({"BlindsPosted", player + " " + blindType, amount});
    }
    void onPhaseChanged(const std::string& phase, const std::string& cards) override {
        events.push_back({"PhaseChanged", phase, 0});
    }
    void onPlayerAction(const std::string& player, const std::string& action, int amount, int, int) override {
        events.push_back({"PlayerAction", player + " " + action, amount});
    }
    void onPotWon(const std::string& winner, int amount, const std::string& rank) override {
        events.push_back({"PotWon", winner + " " + rank, amount});
    }
    void onSplitPot(const std::vector<std::string>& winners, int share, const std::string& rank) override {
        std::string names;
        for (size_t i = 0; i < winners.size(); i++) {
            if (i > 0) names += ",";
            names += winners[i];
        }
        events.push_back({"SplitPot", names + " " + rank, share});
    }
    void onShowdownReveal(const std::string& player, const std::string& cards, const std::string& rank) override {
        events.push_back({"ShowdownReveal", player + " " + rank, 0});
    }
    void onShowdownBegin(const std::string& cards) override {
        events.push_back({"ShowdownBegin", cards, 0});
    }
    void onBetPlaced(const std::string& player, int amount) override {
        events.push_back({"BetPlaced", player, amount});
    }
    void onDealingStarted() override {
        events.push_back({"DealingStarted", "", 0});
    }
    void onPlayerDealt(const std::string& player, const std::string& cards, int value) override {
        events.push_back({"PlayerDealt", player, value});
    }
    void onDealerDealt(const std::string& card) override {
        events.push_back({"DealerDealt", card, 0});
    }
    void onPlayerHit(const std::string& player, const std::string& cards, int value) override {
        events.push_back({"PlayerHit", player, value});
    }
    void onPlayerStand(const std::string& player, int value) override {
        events.push_back({"PlayerStand", player, value});
    }
    void onPlayerBust(const std::string& player) override {
        events.push_back({"PlayerBust", player, 0});
    }
    void onPlayerDoubleDown(const std::string& player, int newBet) override {
        events.push_back({"PlayerDoubleDown", player, newBet});
    }
    void onDealerReveals(const std::string& cards, int value) override {
        events.push_back({"DealerReveals", cards, value});
    }
    void onDealerHits(const std::string& cards, int value) override {
        events.push_back({"DealerHits", cards, value});
    }
    void onDealerBust() override {
        events.push_back({"DealerBust", "", 0});
    }
    void onDealerStand(int value) override {
        events.push_back({"DealerStand", "", value});
    }
    void onBlackjackResult(const std::string& player, const std::string& outcome, int chips) override {
        events.push_back({"BlackjackResult", player + " " + outcome, chips});
    }
    void onPlayerBustedOut(const std::string& player) override {
        events.push_back({"PlayerBustedOut", player, 0});
    }
    void onError(const std::string& message) override {
        events.push_back({"Error", message, 0});
    }

    bool hasEvent(const std::string& type) const {
        for (const auto& e : events) {
            if (e.type == type) return true;
        }
        return false;
    }

    int countEvents(const std::string& type) const {
        int count = 0;
        for (const auto& e : events) {
            if (e.type == type) count++;
        }
        return count;
    }
};

#endif

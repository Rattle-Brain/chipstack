#pragma once

#include "core/blackjack/blackjack_observer.hpp"
#include "core/blackjack/blackjack_player.hpp"
#include "core/common/text.hpp"
#include "core/poker/poker_observer.hpp"
#include "core/poker/poker_player.hpp"
#include <algorithm>
#include <queue>
#include <string>
#include <vector>

template <class T>
T popOr(std::queue<T>& queue, T fallback) {
	if (queue.empty()) return fallback;
	T front = queue.front();
	queue.pop();
	return front;
}

class ScriptedPlayer : public PokerPlayer, public BlackjackPlayer {
public:
	ScriptedPlayer(const std::string& name, int chips) : Player(name, chips) {}
	void queuePokerAction(PokerAction action) { pokerActions.push(action); }
	void queueBlackjackAction(BlackjackAction a) { blackjackActions.push(a); }
	void queueBlackjackBet(int bet) { blackjackBets.push(bet); }

	PokerAction decidePokerAction(const BettingContext&) override {
		return popOr(pokerActions, PokerAction::fold());
	}

	BlackjackAction decideBlackjackAction(const BlackjackContext&) override {
		return popOr(blackjackActions, BlackjackAction::stand());
	}

	int decideBlackjackBet(const BlackjackBetContext& context) override {
		return popOr(blackjackBets, context.minBet);
	}

private:
	std::queue<PokerAction> pokerActions;
	std::queue<BlackjackAction> blackjackActions;
	std::queue<int> blackjackBets;
};

struct RecordedEvent {
	std::string type;
	std::string detail;
	int amount = 0;
};

class RecordingObserver : public PokerObserver, public BlackjackObserver {
public:
	std::vector<RecordedEvent> events;

	bool hasEvent(const std::string& type) const { return countEvents(type) > 0; }

	int countEvents(const std::string& type) const {
		return std::count_if(events.begin(), events.end(), [&](const auto& e) {
			return e.type == type;
		});
	}

	void onHandStarted(const Player& button, int small, int) override {
		events.push_back({"HandStarted", button.getName(), small});
	}

	void onHoleCardsDealt(const Player& player) override {
		std::string cards = formatCards(player.getHand());
		events.push_back({"PlayerCards", player.getName() + ": " + cards});
	}

	void onBlindPosted(
			const Player& player, int amount, const std::string& blind, int
	) override {
		events.push_back({"BlindsPosted", player.getName() + " " + blind, amount});
	}

	void onStreetStarted(const std::string& street, const Cards&) override {
		events.push_back({"PhaseChanged", street});
	}

	void onPlayerAction(
			const Player& player, const PokerAction& action, int
	) override {
		std::string detail = player.getName() + " " + action.getActionName();
		events.push_back({"PlayerAction", detail, action.amount});
	}

	void onPotWon(
			const Player& winner, int amount, const std::string& reason
	) override {
		events.push_back({"PotWon", winner.getName() + " " + reason, amount});
	}

	void onSplitPot(
			const std::vector<const Player*>& winners, int share,
			const std::string& hand
	) override {
		std::vector<std::string> names;
		for (const Player* winner : winners) {
			names.push_back(winner->getName());
		}
		events.push_back({"SplitPot", join(names, ",") + " " + hand, share});
	}

	void onShowdownReveal(
			const Player& player, const std::string& hand
	) override {
		events.push_back({"ShowdownReveal", player.getName() + " " + hand});
	}

	void onShowdownBegin(const Cards& cards) override {
		events.push_back({"ShowdownBegin", formatCards(cards)});
	}

	void onBetPlaced(const Player& player, int amount) override {
		events.push_back({"BetPlaced", player.getName(), amount});
	}

	void onDealingStarted() override { events.push_back({"DealingStarted"}); }

	void onPlayerDealt(const Player& player, int total) override {
		events.push_back({"PlayerDealt", player.getName(), total});
	}

	void onDealerDealt(const CardHolder& dealer) override {
		events.push_back({"DealerDealt", dealer.getHand().front().toString()});
	}

	void onPlayerHit(const Player& player, int total) override {
		events.push_back({"PlayerHit", player.getName(), total});
	}

	void onPlayerStand(const Player& player, int total) override {
		events.push_back({"PlayerStand", player.getName(), total});
	}

	void onPlayerBust(const Player& player) override {
		events.push_back({"PlayerBust", player.getName()});
	}

	void onPlayerDoubleDown(const Player& player, int newBet) override {
		events.push_back({"PlayerDoubleDown", player.getName(), newBet});
	}

	void onDealerReveals(const CardHolder& dealer, int total) override {
		events.push_back({"DealerReveals", formatCards(dealer.getHand()), total});
	}

	void onDealerHits(const CardHolder& dealer, int total) override {
		events.push_back({"DealerHits", formatCards(dealer.getHand()), total});
	}

	void onDealerBust() override { events.push_back({"DealerBust"}); }

	void onDealerStand(int total) override {
		events.push_back({"DealerStand", "", total});
	}

	void onBetSettled(
			const Player& player, const std::string& outcome, int delta
	) override {
		std::string detail = player.getName() + " " + outcome;
		events.push_back({"BlackjackResult", detail, delta});
	}

	void onPlayerBustedOut(const Player& player) override {
		events.push_back({"PlayerBustedOut", player.getName()});
	}

	void onError(const std::string& message) override {
		events.push_back({"Error", message});
	}
};

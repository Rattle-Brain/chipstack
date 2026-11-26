#pragma once

#include "core/table/table_observer.hpp"
#include "poker_action.hpp"
#include <vector>

class PokerObserver : public TableObserver {
public:
	virtual void onHandStarted(
			const Player& buttonPlayer, int smallBlind, int bigBlind
	) {}

	virtual void onHoleCardsDealt(const Player& player) {}

	virtual void onBlindPosted(
			const Player& player, int amount, const std::string& blindName, int pot
	) {}

	virtual void onStreetStarted(
			const std::string& streetName, const Cards& communityCards
	) {}

	virtual void onPlayerAction(
			const Player& player, const PokerAction& action, int pot
	) {}

	virtual void onShowdownBegin(const Cards& communityCards) {}

	virtual void onShowdownReveal(
			const Player& player, const std::string& handName
	) {}

	virtual void onPotWon(
			const Player& winner, int amount, const std::string& reason
	) {}

	virtual void onSplitPot(
			const std::vector<const Player*>& winners, int share,
			const std::string& handName
	) {}
};

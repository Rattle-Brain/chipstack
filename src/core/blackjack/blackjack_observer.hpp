#pragma once

#include "core/table/table_observer.hpp"

class BlackjackObserver : public TableObserver {
public:
	virtual void onBetPlaced(const Player& player, int amount) {}
	virtual void onDealingStarted() {}
	virtual void onPlayerDealt(const Player& player, int handTotal) {}
	virtual void onDealerDealt(const CardHolder& dealer) {}
	virtual void onPlayerHit(const Player& player, int handTotal) {}
	virtual void onPlayerStand(const Player& player, int handTotal) {}
	virtual void onPlayerBust(const Player& player) {}
	virtual void onPlayerDoubleDown(const Player& player, int newBet) {}
	virtual void onDealerReveals(const CardHolder& dealer, int handTotal) {}
	virtual void onDealerHits(const CardHolder& dealer, int handTotal) {}
	virtual void onDealerBust() {}
	virtual void onDealerStand(int handTotal) {}

	virtual void onBetSettled(
			const Player& player, const std::string& outcome, int chipDelta
	) {}
};

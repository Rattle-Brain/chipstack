#pragma once

#include "core/players/dealer.hpp"
#include "table_observer.hpp"
#include <algorithm>
#include <vector>

class TableBase {
public:
	virtual ~TableBase() = default;

	virtual void playHand() = 0;
	virtual std::vector<const Player*> getSeatedPlayers() const = 0;
	virtual void removeBustedPlayers() = 0;

	int getPlayerCount() const { return getSeatedPlayers().size(); }
};

template <class PlayerT, class ObserverT>
class Table : public TableBase {
public:
	using PlayerType = PlayerT;

	void addPlayer(PlayerT* player) { players.push_back(player); }
	const std::vector<PlayerT*>& getPlayers() const { return players; }

	void setObserver(ObserverT* newObserver) {
		observer = newObserver ? newObserver : &silentObserver;
	}

	std::vector<const Player*> getSeatedPlayers() const override {
		return {players.begin(), players.end()};
	}

	void removeBustedPlayers() override {
		auto isBusted = [this](PlayerT* player) {
			if (!player->isBustedOut()) return false;
			observer->onPlayerBustedOut(*player);
			return true;
		};
		players.erase(
				std::remove_if(players.begin(), players.end(), isBusted), players.end()
		);
	}

protected:
	bool reportError(const std::string& message) {
		observer->onError(message);
		return false;
	}

	void returnCardsToDealer() {
		for (PlayerT* player : players) {
			player->clearHand();
		}
		dealer.clearHand();
		dealer.restackDeck();
	}

	std::vector<PlayerT*> players;
	Dealer dealer;
	ObserverT* observer = &silentObserver;

private:
	ObserverT silentObserver;
};

#pragma once

#include "blackjack_observer.hpp"
#include "blackjack_player.hpp"
#include "core/table/table.hpp"
#include <map>

enum class BlackjackState {
	WaitingForPlayers,
	PlayerTurns,
	DealerTurn,
	Resolving,
	HandComplete
};

class BlackjackTable : public Table<BlackjackPlayer, BlackjackObserver> {
public:
	BlackjackTable(int minBet = 10, int maxBet = 500);

	void playHand() override;

	BlackjackState getState() const { return state; }

private:
	void collectBets();
	void collectBet(BlackjackPlayer& player);
	BlackjackBetContext buildBetContext(const Player& player) const;
	void deal();
	void announceDeal();
	void playPlayerTurns();
	void playTurn(BlackjackPlayer& player);
	void applyAction(
			BlackjackPlayer& player, BlackjackAction action, bool canDoubleDown
	);
	void doubleDown(BlackjackPlayer& player, bool allowed);
	void hitCurrentPlayer();
	void standCurrentPlayer();
	void advancePlayer();
	BlackjackPlayer* getCurrentPlayer() const;
	BlackjackContext buildContext(const BlackjackPlayer& player) const;
	void playDealerTurn();
	void announceDealerResult(int dealerTotal);
	void settleBets();
	void collectCards();

	int minBet;
	int maxBet;
	BlackjackState state = BlackjackState::WaitingForPlayers;
	size_t currentPlayerIndex = 0;
	std::map<const Player*, int> bets;
};

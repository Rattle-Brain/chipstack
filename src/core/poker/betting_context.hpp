#pragma once

#include "core/cards/card.hpp"

enum class BettingRoundType { PreFlop, Flop, Turn, River };

enum class Position { EarlyBlind, Early, Middle, Late, Dealer };

struct Blinds {
	int small;
	int big;
};

struct BettingContext {
	BettingRoundType roundType;
	Cards communityCards;
	int pot;
	int currentBet;
	int playerCurrentBet;
	int costToCall;
	int playerChips;
	int numActivePlayers;
	int numPlayersToAct;
	int minRaise;
	int bigBlind;
	int dealerButton;
	int playerSeatIndex;
	int totalPlayers;
	Position position;
};

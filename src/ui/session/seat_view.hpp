#pragma once

#include "core/cards/card.hpp"
#include <string>

struct SeatView {
	std::string name;
	int chips = 0;
	Cards cards;
	bool showCards = false;
	int bet = 0;
	std::string lastAction;
	bool isFolded = false;
	bool isAllIn = false;
	bool isCurrentTurn = false;
	bool isHuman = false;
	bool isDealer = false;
	bool isThinking = false;
};

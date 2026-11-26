#pragma once

#include "core/cards/card.hpp"

namespace BlackjackRules {

constexpr int BLACKJACK = 21;
constexpr int DEALER_STAND_TOTAL = 17;

struct HandValue {
	int total;
	bool soft;
};

struct Settlement {
	const char* outcome;
	int chipDelta;
};

int cardValue(const Card& card);
HandValue evaluate(const Cards& hand);
int handTotal(const Cards& hand);
bool isBust(const Cards& hand);
bool isNatural(const Cards& hand);
Settlement settle(const Cards& playerHand, const Cards& dealerHand, int bet);

} // namespace BlackjackRules

#include "blackjack_rules.hpp"
#include <algorithm>

namespace BlackjackRules {

int cardValue(const Card& card) {
	if (card.getRank() == ACE_RANK) return 11;
	return std::min(card.getRank(), 10);
}

HandValue evaluate(const Cards& hand) {
	int total = 0;
	int softAces = 0;
	for (const Card& card : hand) {
		total += cardValue(card);
		if (card.getRank() == ACE_RANK) softAces++;
	}
	while (total > BLACKJACK && softAces > 0) {
		total -= 10;
		softAces--;
	}
	return {total, softAces > 0};
}

int handTotal(const Cards& hand) {
	return evaluate(hand).total;
}

bool isBust(const Cards& hand) {
	return handTotal(hand) > BLACKJACK;
}

bool isNatural(const Cards& hand) {
	return hand.size() == 2 && handTotal(hand) == BLACKJACK;
}

Settlement settle(const Cards& playerHand, const Cards& dealerHand, int bet) {
	int playerTotal = handTotal(playerHand);
	int dealerTotal = handTotal(dealerHand);
	if (playerTotal > BLACKJACK) return {"BUST", -bet};
	if (isNatural(playerHand) && !isNatural(dealerHand)) {
		return {"BLACKJACK", bet * 3 / 2};
	}
	if (dealerTotal > BLACKJACK || playerTotal > dealerTotal) return {"WIN", bet};
	if (playerTotal == dealerTotal) return {"PUSH", 0};
	return {"LOSE", -bet};
}

} // namespace BlackjackRules

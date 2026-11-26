#include "hand_strength.hpp"
#include "core/poker/hand_evaluator.hpp"
#include <algorithm>
#include <array>

namespace {

struct PremiumHand {
	int high;
	int low;
	float suited;
	float offsuit;
};

const PremiumHand PREMIUM_HANDS[] = {
		{14, 13, 0.92f, 0.88f}, {14, 12, 0.86f, 0.82f}, {14, 11, 0.80f, 0.76f},
		{14, 10, 0.76f, 0.70f}, {13, 12, 0.82f, 0.75f}, {13, 11, 0.76f, 0.70f},
};

const float RANK_STRENGTH[] = {
		0.12f, 0.35f, 0.55f, 0.68f, 0.80f, 0.85f, 0.90f, 0.94f, 0.97f, 1.00f,
};

using RankPresence = std::array<bool, ACE_RANK + 1>;

const PremiumHand* findPremiumHand(int high, int low) {
	for (const PremiumHand& hand : PREMIUM_HANDS) {
		if (hand.high == high && hand.low == low) return &hand;
	}
	return nullptr;
}

float unpairedStrength(int high, int low, bool suited) {
	int gap = high - low;
	float strength = 0.10f + (high - 2) * 0.0375f - (gap - 1) * 0.03f;
	if (gap == 1 && high >= 5) strength += 0.04f;
	if (gap == 2 && high >= 6) strength += 0.02f;
	if (suited) strength += 0.06f;
	return strength + (low - 2) * 0.01f;
}

RankPresence rankPresence(const Cards& cards) {
	RankPresence presence{};
	for (const Card& card : cards) {
		presence[card.getRank()] = true;
	}
	presence[1] = presence[ACE_RANK]; // Ace also plays low
	return presence;
}

int countInWindow(const RankPresence& presence, int lowestRank) {
	return std::count(
			presence.begin() + lowestRank, presence.begin() + lowestRank + 5, true
	);
}

bool hasFlushDraw(const Cards& cards) {
	std::array<int, SUIT_COUNT> suitCounts{};
	for (const Card& card : cards) {
		suitCounts[card.getSuit()]++;
	}
	return std::count(suitCounts.begin(), suitCounts.end(), 4) > 0;
}

int straightDrawOuts(const Cards& cards) {
	RankPresence presence = rankPresence(cards);
	int outs = 0;
	for (int lowest = 1; lowest <= 10; lowest++) {
		if (countInWindow(presence, lowest) != 4) continue;
		bool missingEdge = !presence[lowest] || !presence[lowest + 4];
		bool openEnded = lowest <= 9 && missingEdge;
		outs = std::max(outs, openEnded ? 8 : 4);
	}
	return outs;
}

} // namespace

float HandStrength::preflopStrength(int rank1, int rank2, bool suited) {
	int high = std::max(rank1, rank2);
	int low = std::min(rank1, rank2);
	if (high == low) return 0.50f + (high - 2) * 0.0417f;
	if (const PremiumHand* premium = findPremiumHand(high, low)) {
		return suited ? premium->suited : premium->offsuit;
	}
	return std::clamp(unpairedStrength(high, low, suited), 0.05f, 0.95f);
}

float HandStrength::postflopStrength(
		const Cards& holeCards, const Cards& communityCards
) {
	Cards cards = combineCards(holeCards, communityCards);
	HandResult result = HandEvaluator::evaluate(cards);
	float kicker = result.values.empty() ? 0 : (result.values[0] - 2) * 0.002f;
	return std::min(1.0f, RANK_STRENGTH[static_cast<int>(result.rank)] + kicker);
}

int HandStrength::countDrawOuts(
		const Cards& holeCards, const Cards& communityCards
) {
	Cards cards = combineCards(holeCards, communityCards);
	if (HandEvaluator::evaluate(cards).rank >= HandRank::Straight) return 0;
	int flushOuts = hasFlushDraw(cards) ? 9 : 0;
	return std::min(15, flushOuts + straightDrawOuts(cards));
}

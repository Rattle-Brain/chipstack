#pragma once

#include "core/cards/card.hpp"
#include <vector>

enum class HandRank {
	HighCard,
	OnePair,
	TwoPair,
	ThreeOfAKind,
	Straight,
	Flush,
	FullHouse,
	FourOfAKind,
	StraightFlush,
	RoyalFlush
};

struct HandResult {
	HandRank rank = HandRank::HighCard;
	std::vector<int> values;

	bool operator<(const HandResult& other) const;

	bool operator>(const HandResult& other) const { return other < *this; }

	bool operator==(const HandResult& other) const;
	const char* getRankName() const;
};

namespace HandEvaluator {

HandResult evaluate(const Cards& cards);

} // namespace HandEvaluator

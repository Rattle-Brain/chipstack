#include "hand_evaluator.hpp"
#include <algorithm>
#include <map>
#include <tuple>

bool HandResult::operator<(const HandResult& other) const {
	return std::tie(rank, values) < std::tie(other.rank, other.values);
}

bool HandResult::operator==(const HandResult& other) const {
	return rank == other.rank && values == other.values;
}

const char* HandResult::getRankName() const {
	static const char* names[] = {"High Card",      "One Pair",
																"Two Pair",       "Three of a Kind",
																"Straight",       "Flush",
																"Full House",     "Four of a Kind",
																"Straight Flush", "Royal Flush"};
	return names[static_cast<int>(rank)];
}

namespace {

constexpr size_t HAND_SIZE = 5;

struct RankGroup {
	int count;
	int rank;
};

std::vector<int> ranksDescending(const Cards& cards) {
	std::vector<int> ranks;
	for (const Card& card : cards) {
		ranks.push_back(card.getRank());
	}
	std::sort(ranks.rbegin(), ranks.rend());
	return ranks;
}

std::vector<RankGroup> groupByRank(const std::vector<int>& ranks) {
	std::map<int, int> counts;
	for (int rank : ranks) {
		counts[rank]++;
	}
	std::vector<RankGroup> groups;
	for (const auto& [rank, count] : counts) {
		groups.push_back({count, rank});
	}
	std::sort(groups.begin(), groups.end(), [](RankGroup a, RankGroup b) {
		return std::tie(a.count, a.rank) > std::tie(b.count, b.rank);
	});
	return groups;
}

bool isFlush(const Cards& cards) {
	return std::all_of(cards.begin(), cards.end(), [&](const Card& card) {
		return card.getSuit() == cards.front().getSuit();
	});
}

int straightHighCard(const std::vector<int>& ranks, size_t distinctRanks) {
	if (distinctRanks != HAND_SIZE) return 0;
	if (ranks.front() - ranks.back() == 4) return ranks.front();
	bool isWheel = ranks.front() == ACE_RANK && ranks[1] == 5;
	return isWheel ? 5 : 0;
}

HandRank rankFromGroups(const std::vector<RankGroup>& groups) {
	int largest = groups[0].count;
	int second = groups.size() > 1 ? groups[1].count : 0;
	if (largest == 4) return HandRank::FourOfAKind;
	if (largest == 3) {
		return second == 2 ? HandRank::FullHouse : HandRank::ThreeOfAKind;
	}
	if (largest == 2) {
		return second == 2 ? HandRank::TwoPair : HandRank::OnePair;
	}
	return HandRank::HighCard;
}

HandResult evaluateFive(const Cards& cards) {
	std::vector<int> ranks = ranksDescending(cards);
	std::vector<RankGroup> groups = groupByRank(ranks);
	bool flush = isFlush(cards);
	int straightHigh = straightHighCard(ranks, groups.size());
	if (flush && straightHigh == ACE_RANK) return {HandRank::RoyalFlush, {}};
	if (flush && straightHigh) return {HandRank::StraightFlush, {straightHigh}};
	if (flush) return {HandRank::Flush, ranks};
	if (straightHigh) return {HandRank::Straight, {straightHigh}};
	std::vector<int> groupedValues;
	for (const RankGroup& group : groups) {
		groupedValues.push_back(group.rank);
	}
	return {rankFromGroups(groups), groupedValues};
}

Cards selectCards(const Cards& cards, const std::vector<bool>& selection) {
	Cards selected;
	for (size_t i = 0; i < cards.size(); i++) {
		if (selection[i]) selected.push_back(cards[i]);
	}
	return selected;
}

} // namespace

HandResult HandEvaluator::evaluate(const Cards& cards) {
	if (cards.size() < HAND_SIZE) return {};
	std::vector<bool> selection(cards.size(), false);
	std::fill(selection.begin(), selection.begin() + HAND_SIZE, true);
	HandResult best;
	do {
		best = std::max(best, evaluateFive(selectCards(cards, selection)));
	} while (std::prev_permutation(selection.begin(), selection.end()));
	return best;
}

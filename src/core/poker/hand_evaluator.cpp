#include "hand_evaluator.hpp"
#include <algorithm>
#include <map>

bool HandResult::operator>(const HandResult& other) const {
  if (rank != other.rank) return rank > other.rank;
  return values > other.values;
}

bool HandResult::operator<(const HandResult& other) const {
  if (rank != other.rank) return rank < other.rank;
  return values < other.values;
}

bool HandResult::operator==(const HandResult& other) const {
  return rank == other.rank && values == other.values;
}

const char* HandResult::getRankName() const {
  switch (rank) {
    case HandRank::HighCard:       return "High Card";
    case HandRank::OnePair:        return "One Pair";
    case HandRank::TwoPair:        return "Two Pair";
    case HandRank::ThreeOfAKind:   return "Three of a Kind";
    case HandRank::Straight:       return "Straight";
    case HandRank::Flush:          return "Flush";
    case HandRank::FullHouse:      return "Full House";
    case HandRank::FourOfAKind:    return "Four of a Kind";
    case HandRank::StraightFlush:  return "Straight Flush";
    case HandRank::RoyalFlush:     return "Royal Flush";
  }
  return "Unknown";
}

HandResult HandEvaluator::evaluateFive(std::vector<Card>& cards) {
  std::map<int, int> rankCount;
  for (size_t i = 0; i < cards.size(); i++) {
    rankCount[cards[i].getRank()]++;
  }

  // Check flush
  bool isFlush = true;
  Suit firstSuit = cards[0].getSuit();
  for (size_t i = 1; i < cards.size(); i++) {
    if (cards[i].getSuit() != firstSuit) {
      isFlush = false;
      break;
    }
  }

  // Collect ranks sorted descending
  std::vector<int> ranks;
  for (size_t i = 0; i < cards.size(); i++) {
    ranks.push_back(cards[i].getRank());
  }
  std::sort(ranks.begin(), ranks.end(), std::greater<int>());

  // Check straight
  bool isStraight = false;
  int straightHigh = 0;

  // Normal straight: 5 consecutive
  if (ranks[0] - ranks[4] == 4 && rankCount.size() == 5) {
    isStraight = true;
    straightHigh = ranks[0];
  }
  // Ace-low straight: A-2-3-4-5
  if (!isStraight && ranks[0] == 14 && ranks[1] == 5 &&
      ranks[2] == 4 && ranks[3] == 3 && ranks[4] == 2) {
    isStraight = true;
    straightHigh = 5;
  }

  // Classify by rank counts
  std::vector<int> fours, threes, pairs, singles;
  for (std::map<int, int>::iterator it = rankCount.begin(); it != rankCount.end(); ++it) {
    if (it->second == 4) fours.push_back(it->first);
    else if (it->second == 3) threes.push_back(it->first);
    else if (it->second == 2) pairs.push_back(it->first);
    else singles.push_back(it->first);
  }
  std::sort(fours.begin(), fours.end(), std::greater<int>());
  std::sort(threes.begin(), threes.end(), std::greater<int>());
  std::sort(pairs.begin(), pairs.end(), std::greater<int>());
  std::sort(singles.begin(), singles.end(), std::greater<int>());

  HandResult result;

  if (isFlush && isStraight) {
    if (straightHigh == 14) {
      result.rank = HandRank::RoyalFlush;
    } else {
      result.rank = HandRank::StraightFlush;
      result.values.push_back(straightHigh);
    }
  } else if (!fours.empty()) {
    result.rank = HandRank::FourOfAKind;
    result.values.push_back(fours[0]);
    result.values.push_back(singles[0]);
  } else if (!threes.empty() && !pairs.empty()) {
    result.rank = HandRank::FullHouse;
    result.values.push_back(threes[0]);
    result.values.push_back(pairs[0]);
  } else if (isFlush) {
    result.rank = HandRank::Flush;
    result.values = ranks;
  } else if (isStraight) {
    result.rank = HandRank::Straight;
    result.values.push_back(straightHigh);
  } else if (!threes.empty()) {
    result.rank = HandRank::ThreeOfAKind;
    result.values.push_back(threes[0]);
    for (size_t i = 0; i < singles.size(); i++) {
      result.values.push_back(singles[i]);
    }
  } else if (pairs.size() >= 2) {
    result.rank = HandRank::TwoPair;
    result.values.push_back(pairs[0]);
    result.values.push_back(pairs[1]);
    result.values.push_back(singles[0]);
  } else if (pairs.size() == 1) {
    result.rank = HandRank::OnePair;
    result.values.push_back(pairs[0]);
    for (size_t i = 0; i < singles.size(); i++) {
      result.values.push_back(singles[i]);
    }
  } else {
    result.rank = HandRank::HighCard;
    result.values = ranks;
  }

  return result;
}

HandResult HandEvaluator::evaluate(std::vector<Card> cards) {
  if (cards.size() == 5) {
    return evaluateFive(cards);
  }

  // For 6-7 cards, find the best 5-card combination
  HandResult best;
  best.rank = HandRank::HighCard;
  bool first = true;
  int n = cards.size();

  for (int a = 0; a < n; a++) {
    for (int b = a + 1; b < n; b++) {
      for (int c = b + 1; c < n; c++) {
        for (int d = c + 1; d < n; d++) {
          for (int e = d + 1; e < n; e++) {
            std::vector<Card> five;
            five.push_back(cards[a]);
            five.push_back(cards[b]);
            five.push_back(cards[c]);
            five.push_back(cards[d]);
            five.push_back(cards[e]);

            HandResult result = evaluateFive(five);
            if (first || result > best) {
              best = result;
              first = false;
            }
          }
        }
      }
    }
  }

  return best;
}

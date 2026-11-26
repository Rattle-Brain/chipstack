#ifndef HAND_EVALUATOR_HPP
#define HAND_EVALUATOR_HPP

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
    HandRank rank;
    std::vector<int> values;

    bool operator>(const HandResult& other) const;
    bool operator<(const HandResult& other) const;
    bool operator==(const HandResult& other) const;

    const char* getRankName() const;
};

class HandEvaluator {
  public:
    static HandResult evaluate(std::vector<Card> cards);

  private:
    static HandResult evaluateFive(std::vector<Card>& cards);
};

#endif

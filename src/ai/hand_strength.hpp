#ifndef HAND_STRENGTH_HPP
#define HAND_STRENGTH_HPP

#include "core/cards/card.hpp"
#include "core/poker/hand_evaluator.hpp"
#include <vector>

class HandStrength {
  public:
    static float preflopStrength(int rank1, int rank2, bool suited);
    static float postflopStrength(const std::vector<Card>& holeCards,
                                  const std::vector<Card>& communityCards);
    static int countDrawOuts(const std::vector<Card>& holeCards,
                             const std::vector<Card>& communityCards);

  private:
    static float handRankToStrength(HandRank rank, const std::vector<int>& values);
    static bool hasFlushDraw(const std::vector<Card>& allCards);
    static bool hasStraightDraw(const std::vector<Card>& allCards);
};

#endif

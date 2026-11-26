#include "hand_strength.hpp"
#include <algorithm>
#include <map>
#include <cmath>

float HandStrength::preflopStrength(int rank1, int rank2, bool suited) {
    int high = std::max(rank1, rank2);
    int low = std::min(rank1, rank2);
    bool paired = (high == low);

    if (paired) {
        // AA=1.0, KK=0.95, QQ=0.90, ..., 22=0.50
        return 0.50f + (high - 2) * 0.0417f;
    }

    // Base from high card (A=0.55, K=0.48, Q=0.42, ...)
    float base = 0.10f + (high - 2) * 0.0375f;

    // Gap penalty: cards further apart are weaker
    int gap = high - low;
    float gapPenalty = (gap - 1) * 0.03f;
    base -= gapPenalty;

    // Connectedness bonus (adjacent cards)
    if (gap == 1 && high >= 5) base += 0.04f;
    if (gap == 2 && high >= 6) base += 0.02f;

    // Suited bonus
    if (suited) base += 0.06f;

    // Low card boost (kicker quality)
    base += (low - 2) * 0.01f;

    // Specific premium hand adjustments
    if (high == 14 && low == 13) base = suited ? 0.92f : 0.88f; // AK
    if (high == 14 && low == 12) base = suited ? 0.86f : 0.82f; // AQ
    if (high == 14 && low == 11) base = suited ? 0.80f : 0.76f; // AJ
    if (high == 14 && low == 10) base = suited ? 0.76f : 0.70f; // AT
    if (high == 13 && low == 12) base = suited ? 0.82f : 0.75f; // KQ
    if (high == 13 && low == 11) base = suited ? 0.76f : 0.70f; // KJ

    // Clamp
    if (base < 0.05f) base = 0.05f;
    if (base > 0.95f) base = 0.95f;

    return base;
}

float HandStrength::handRankToStrength(HandRank rank, const std::vector<int>& values) {
    float base;
    switch (rank) {
        case HandRank::RoyalFlush:     return 1.00f;
        case HandRank::StraightFlush:  base = 0.97f; break;
        case HandRank::FourOfAKind:    base = 0.94f; break;
        case HandRank::FullHouse:      base = 0.90f; break;
        case HandRank::Flush:          base = 0.85f; break;
        case HandRank::Straight:       base = 0.80f; break;
        case HandRank::ThreeOfAKind:   base = 0.68f; break;
        case HandRank::TwoPair:        base = 0.55f; break;
        case HandRank::OnePair:        base = 0.35f; break;
        case HandRank::HighCard:
        default:                       base = 0.12f; break;
    }

    // Kicker quality adjustment within the rank
    if (!values.empty()) {
        float kickerBonus = (values[0] - 2) * 0.002f;
        base += kickerBonus;
    }

    if (base > 1.0f) base = 1.0f;
    return base;
}

float HandStrength::postflopStrength(const std::vector<Card>& holeCards,
                                     const std::vector<Card>& communityCards) {
    std::vector<Card> allCards;
    for (size_t i = 0; i < holeCards.size(); i++) {
        allCards.push_back(holeCards[i]);
    }
    for (size_t i = 0; i < communityCards.size(); i++) {
        allCards.push_back(communityCards[i]);
    }

    HandResult result = HandEvaluator::evaluate(allCards);
    return handRankToStrength(result.rank, result.values);
}

bool HandStrength::hasFlushDraw(const std::vector<Card>& allCards) {
    int suitCounts[4] = {0, 0, 0, 0};
    for (size_t i = 0; i < allCards.size(); i++) {
        suitCounts[allCards[i].getSuit()]++;
    }
    for (int i = 0; i < 4; i++) {
        if (suitCounts[i] == 4) return true;
    }
    return false;
}

bool HandStrength::hasStraightDraw(const std::vector<Card>& allCards) {
    bool ranks[15] = {};
    for (size_t i = 0; i < allCards.size(); i++) {
        ranks[allCards[i].getRank()] = true;
    }
    // Ace can also be low
    if (ranks[14]) ranks[1] = true;

    // Check for 4 in a row (open-ended or gutshot)
    for (int start = 1; start <= 10; start++) {
        int count = 0;
        for (int j = start; j < start + 5; j++) {
            if (ranks[j]) count++;
        }
        if (count == 4) return true;
    }
    return false;
}

int HandStrength::countDrawOuts(const std::vector<Card>& holeCards,
                                const std::vector<Card>& communityCards) {
    std::vector<Card> allCards;
    for (size_t i = 0; i < holeCards.size(); i++) {
        allCards.push_back(holeCards[i]);
    }
    for (size_t i = 0; i < communityCards.size(); i++) {
        allCards.push_back(communityCards[i]);
    }

    // Check if we already have a made hand (straight or better)
    HandResult current = HandEvaluator::evaluate(allCards);
    if (current.rank >= HandRank::Straight) return 0;

    int outs = 0;
    if (hasFlushDraw(allCards)) outs += 9;
    if (hasStraightDraw(allCards)) {
        // Open-ended = 8 outs, gutshot = 4 outs
        // Simplified: count gaps
        bool ranks[15] = {};
        for (size_t i = 0; i < allCards.size(); i++) {
            ranks[allCards[i].getRank()] = true;
        }
        if (ranks[14]) ranks[1] = true;

        bool openEnded = false;
        for (int start = 1; start <= 9; start++) {
            int count = 0;
            for (int j = start; j < start + 5; j++) {
                if (ranks[j]) count++;
            }
            if (count == 4) {
                // Check if it's open-ended (missing at either end)
                if (!ranks[start] || !ranks[start + 4]) {
                    openEnded = true;
                }
            }
        }
        outs += openEnded ? 8 : 4;
    }

    // Avoid double-counting flush+straight combo outs
    if (outs > 15) outs = 15;

    return outs;
}

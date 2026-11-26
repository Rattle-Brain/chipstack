#include <gtest/gtest.h>
#include "ai/hand_strength.hpp"

TEST(HandStrengthTest, PocketAcesStrongest) {
    float strength = HandStrength::preflopStrength(14, 14, false);
    EXPECT_NEAR(strength, 1.0f, 0.01f);
}

TEST(HandStrengthTest, LowOffSuitWeakest) {
    float strength72 = HandStrength::preflopStrength(7, 2, false);
    float strengthAA = HandStrength::preflopStrength(14, 14, false);
    EXPECT_LT(strength72, strengthAA);
    EXPECT_LT(strength72, 0.4f);
}

TEST(HandStrengthTest, SuitedBetterThanUnsuited) {
    float suited = HandStrength::preflopStrength(14, 13, true);
    float unsuited = HandStrength::preflopStrength(14, 13, false);
    EXPECT_GT(suited, unsuited);
}

TEST(HandStrengthTest, PairsMonotonic) {
    float aa = HandStrength::preflopStrength(14, 14, false);
    float kk = HandStrength::preflopStrength(13, 13, false);
    float qq = HandStrength::preflopStrength(12, 12, false);
    float jj = HandStrength::preflopStrength(11, 11, false);
    float tt = HandStrength::preflopStrength(10, 10, false);

    EXPECT_GE(aa, kk);
    EXPECT_GE(kk, qq);
    EXPECT_GE(qq, jj);
    EXPECT_GE(jj, tt);
}

TEST(HandStrengthTest, PostflopWithFlush) {
    std::vector<Card> hole = {Card(14, Spades), Card(8, Spades)};
    std::vector<Card> community = {Card(10, Spades), Card(5, Spades), Card(3, Spades)};
    float strength = HandStrength::postflopStrength(hole, community);
    EXPECT_GT(strength, 0.7f);
}

TEST(HandStrengthTest, PostflopHighCard) {
    std::vector<Card> hole = {Card(7, Spades), Card(2, Hearts)};
    std::vector<Card> community = {Card(14, Clubs), Card(10, Diamonds), Card(5, Hearts)};
    float strength = HandStrength::postflopStrength(hole, community);
    EXPECT_LT(strength, 0.3f);
}

TEST(HandStrengthTest, FlushDrawOuts) {
    std::vector<Card> hole = {Card(14, Spades), Card(8, Spades)};
    std::vector<Card> community = {Card(10, Spades), Card(5, Spades), Card(3, Hearts)};
    int outs = HandStrength::countDrawOuts(hole, community);
    EXPECT_GE(outs, 8);
}

TEST(HandStrengthTest, MadeHandNoDrawOuts) {
    std::vector<Card> hole = {Card(14, Spades), Card(8, Spades)};
    std::vector<Card> community = {Card(10, Spades), Card(5, Spades), Card(3, Spades)};
    int outs = HandStrength::countDrawOuts(hole, community);
    EXPECT_EQ(outs, 0);
}

TEST(HandStrengthTest, PreflopSymmetric) {
    float ak = HandStrength::preflopStrength(14, 13, false);
    float ka = HandStrength::preflopStrength(13, 14, false);
    EXPECT_FLOAT_EQ(ak, ka);
}

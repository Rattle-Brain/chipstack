#include "core/poker/hand_evaluator.hpp"
#include <gtest/gtest.h>

static std::vector<Card> makeHand(std::vector<std::pair<int, Suit>> cards) {
	std::vector<Card> hand;
	for (auto& [rank, suit] : cards) {
		hand.push_back(Card(rank, suit));
	}
	return hand;
}

TEST(HandEvaluatorTest, RoyalFlush) {
	auto hand = makeHand({
			{14, Spades},
			{13, Spades},
			{12, Spades},
			{11, Spades},
			{10, Spades},
	});
	HandResult r = HandEvaluator::evaluate(hand);
	EXPECT_EQ(r.rank, HandRank::RoyalFlush);
}

TEST(HandEvaluatorTest, StraightFlush) {
	auto hand = makeHand({
			{9, Hearts},
			{8, Hearts},
			{7, Hearts},
			{6, Hearts},
			{5, Hearts},
	});
	HandResult r = HandEvaluator::evaluate(hand);
	EXPECT_EQ(r.rank, HandRank::StraightFlush);
}

TEST(HandEvaluatorTest, FourOfAKind) {
	auto hand = makeHand({
			{10, Spades},
			{10, Hearts},
			{10, Clubs},
			{10, Diamonds},
			{5, Spades},
	});
	HandResult r = HandEvaluator::evaluate(hand);
	EXPECT_EQ(r.rank, HandRank::FourOfAKind);
}

TEST(HandEvaluatorTest, FullHouse) {
	auto hand = makeHand({
			{8, Spades},
			{8, Hearts},
			{8, Clubs},
			{3, Diamonds},
			{3, Spades},
	});
	HandResult r = HandEvaluator::evaluate(hand);
	EXPECT_EQ(r.rank, HandRank::FullHouse);
}

TEST(HandEvaluatorTest, Flush) {
	auto hand = makeHand({
			{14, Diamonds},
			{10, Diamonds},
			{7, Diamonds},
			{4, Diamonds},
			{2, Diamonds},
	});
	HandResult r = HandEvaluator::evaluate(hand);
	EXPECT_EQ(r.rank, HandRank::Flush);
}

TEST(HandEvaluatorTest, Straight) {
	auto hand = makeHand({
			{9, Spades},
			{8, Hearts},
			{7, Clubs},
			{6, Diamonds},
			{5, Spades},
	});
	HandResult r = HandEvaluator::evaluate(hand);
	EXPECT_EQ(r.rank, HandRank::Straight);
}

TEST(HandEvaluatorTest, AceLowStraight) {
	auto hand = makeHand({
			{14, Spades},
			{2, Hearts},
			{3, Clubs},
			{4, Diamonds},
			{5, Spades},
	});
	HandResult r = HandEvaluator::evaluate(hand);
	EXPECT_EQ(r.rank, HandRank::Straight);
	EXPECT_EQ(r.values[0], 5);
}

TEST(HandEvaluatorTest, ThreeOfAKind) {
	auto hand = makeHand({
			{7, Spades},
			{7, Hearts},
			{7, Clubs},
			{12, Diamonds},
			{3, Spades},
	});
	HandResult r = HandEvaluator::evaluate(hand);
	EXPECT_EQ(r.rank, HandRank::ThreeOfAKind);
}

TEST(HandEvaluatorTest, TwoPair) {
	auto hand = makeHand({
			{14, Spades},
			{14, Hearts},
			{8, Clubs},
			{8, Diamonds},
			{3, Spades},
	});
	HandResult r = HandEvaluator::evaluate(hand);
	EXPECT_EQ(r.rank, HandRank::TwoPair);
}

TEST(HandEvaluatorTest, OnePair) {
	auto hand = makeHand({
			{10, Spades},
			{10, Hearts},
			{8, Clubs},
			{5, Diamonds},
			{2, Spades},
	});
	HandResult r = HandEvaluator::evaluate(hand);
	EXPECT_EQ(r.rank, HandRank::OnePair);
}

TEST(HandEvaluatorTest, HighCard) {
	auto hand = makeHand({
			{14, Spades},
			{10, Hearts},
			{8, Clubs},
			{5, Diamonds},
			{2, Hearts},
	});
	HandResult r = HandEvaluator::evaluate(hand);
	EXPECT_EQ(r.rank, HandRank::HighCard);
}

TEST(HandEvaluatorTest, RankOrdering) {
	auto royalFlush = HandEvaluator::evaluate(makeHand(
			{{14, Spades}, {13, Spades}, {12, Spades}, {11, Spades}, {10, Spades}}
	));
	auto straight = HandEvaluator::evaluate(makeHand(
			{{9, Spades}, {8, Hearts}, {7, Clubs}, {6, Diamonds}, {5, Spades}}
	));
	auto pair = HandEvaluator::evaluate(makeHand(
			{{10, Spades}, {10, Hearts}, {8, Clubs}, {5, Diamonds}, {2, Spades}}
	));
	auto highCard = HandEvaluator::evaluate(makeHand(
			{{14, Spades}, {10, Hearts}, {8, Clubs}, {5, Diamonds}, {2, Hearts}}
	));

	EXPECT_TRUE(royalFlush > straight);
	EXPECT_TRUE(straight > pair);
	EXPECT_TRUE(pair > highCard);
	EXPECT_FALSE(highCard > pair);
}

TEST(HandEvaluatorTest, SevenCardEvaluation) {
	auto hand = makeHand({
			{14, Spades},
			{13, Spades},
			{12, Spades},
			{11, Spades},
			{10, Spades},
			{2, Hearts},
			{3, Clubs},
	});
	HandResult r = HandEvaluator::evaluate(hand);
	EXPECT_EQ(r.rank, HandRank::RoyalFlush);
}

TEST(HandEvaluatorTest, SevenCardBestFive) {
	auto hand = makeHand({
			{8, Spades},
			{8, Hearts},
			{8, Clubs},
			{3, Diamonds},
			{3, Spades},
			{5, Hearts},
			{7, Clubs},
	});
	HandResult r = HandEvaluator::evaluate(hand);
	EXPECT_EQ(r.rank, HandRank::FullHouse);
}

TEST(HandEvaluatorTest, TieBreakingKickers) {
	auto hand1 = HandEvaluator::evaluate(makeHand(
			{{14, Spades}, {14, Hearts}, {13, Clubs}, {10, Diamonds}, {5, Spades}}
	));
	auto hand2 = HandEvaluator::evaluate(makeHand(
			{{14, Clubs}, {14, Diamonds}, {12, Spades}, {10, Hearts}, {5, Hearts}}
	));
	EXPECT_TRUE(hand1 > hand2);
}

TEST(HandEvaluatorTest, EqualHands) {
	auto hand1 = HandEvaluator::evaluate(makeHand(
			{{14, Spades}, {13, Hearts}, {12, Clubs}, {11, Diamonds}, {10, Hearts}}
	));
	auto hand2 = HandEvaluator::evaluate(makeHand(
			{{14, Hearts}, {13, Clubs}, {12, Diamonds}, {11, Spades}, {10, Clubs}}
	));
	EXPECT_TRUE(hand1 == hand2);
}

TEST(HandEvaluatorTest, GetRankName) {
	auto hand = HandEvaluator::evaluate(makeHand(
			{{14, Spades}, {13, Spades}, {12, Spades}, {11, Spades}, {10, Spades}}
	));
	EXPECT_STREQ(hand.getRankName(), "Royal Flush");
}

TEST(HandEvaluatorTest, AceHighStraight) {
	auto hand = makeHand({
			{14, Spades},
			{13, Hearts},
			{12, Clubs},
			{11, Diamonds},
			{10, Spades},
	});
	HandResult r = HandEvaluator::evaluate(hand);
	EXPECT_EQ(r.rank, HandRank::Straight);
}

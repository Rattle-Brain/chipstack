#include "core/cards/deck.hpp"
#include <gtest/gtest.h>
#include <map>
#include <set>
#include <string>

TEST(DeckTest, Has52Cards) {
	Deck deck;
	std::set<std::string> cards;
	for (int i = 0; i < 52; i++) {
		Card c = deck.drawCard();
		cards.insert(c.toString());
	}
	EXPECT_EQ(cards.size(), 52u);
}

TEST(DeckTest, AllRanksPresent) {
	Deck deck;
	std::set<int> ranks;
	for (int i = 0; i < 52; i++) {
		Card c = deck.drawCard();
		ranks.insert(c.getRank());
	}
	EXPECT_EQ(ranks.size(), 13u);
	for (int r = 2; r <= 14; r++) {
		EXPECT_TRUE(ranks.count(r) > 0) << "Missing rank " << r;
	}
}

TEST(DeckTest, AllSuitsPresent) {
	Deck deck;
	std::set<int> suits;
	for (int i = 0; i < 52; i++) {
		Card c = deck.drawCard();
		suits.insert(c.getSuit());
	}
	EXPECT_EQ(suits.size(), 4u);
}

TEST(DeckTest, ResetIndexAllowsRepull) {
	Deck deck;
	Card first = deck.drawCard();
	deck.restack();
	Card afterReset = deck.drawCard();
	EXPECT_EQ(first.getRank(), afterReset.getRank());
	EXPECT_EQ(first.getSuit(), afterReset.getSuit());
}

TEST(DeckTest, ShuffleChangesOrder) {
	Deck deck;
	std::vector<std::string> before;
	for (int i = 0; i < 10; i++) {
		before.push_back(deck.drawCard().toString());
	}

	deck.restack();
	deck.shuffle();
	deck.restack();

	std::vector<std::string> after;
	for (int i = 0; i < 10; i++) {
		after.push_back(deck.drawCard().toString());
	}

	bool anyDifferent = false;
	for (int i = 0; i < 10; i++) {
		if (before[i] != after[i]) {
			anyDifferent = true;
			break;
		}
	}
	EXPECT_TRUE(anyDifferent) << "Shuffle should change card order";
}

TEST(DeckTest, EachRankAppearsFourTimes) {
	Deck deck;
	std::map<int, int> rankCount;
	for (int i = 0; i < 52; i++) {
		Card c = deck.drawCard();
		rankCount[c.getRank()]++;
	}
	for (int r = 2; r <= 14; r++) {
		EXPECT_EQ(rankCount[r], 4) << "Rank " << r << " should appear 4 times";
	}
}

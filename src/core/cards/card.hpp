#pragma once

#include <string>
#include <vector>

enum Suit { Spades, Hearts, Clubs, Diamonds };

constexpr int SUIT_COUNT = 4;
constexpr int LOWEST_RANK = 2;
constexpr int JACK_RANK = 11;
constexpr int ACE_RANK = 14;

class Card {
public:
	Card() = default;
	Card(int rank, Suit suit);

	int getRank() const { return rank; }
	Suit getSuit() const { return suit; }

	bool isRed() const;
	std::string rankLabel() const;
	std::string suitSymbol() const;
	std::string toString() const;

private:
	int rank = 1;
	Suit suit = Spades;
};

using Cards = std::vector<Card>;

std::vector<std::string> cardLabels(const Cards& cards);
std::string formatCards(const Cards& cards);
Cards combineCards(const Cards& first, const Cards& second);

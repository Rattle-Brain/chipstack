#pragma once

#include "card.hpp"
#include <array>
#include <random>

class Deck {
public:
	static constexpr int SIZE = 52;

	Deck();
	Card drawCard();
	void shuffle();
	void restack();

private:
	std::array<Card, SIZE> cards;
	int topIndex = SIZE - 1;
	std::mt19937 generator;
};

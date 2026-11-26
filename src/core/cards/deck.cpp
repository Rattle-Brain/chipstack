#include "deck.hpp"
#include <algorithm>

Deck::Deck() : generator(std::random_device{}()) {
	int index = 0;
	for (int suit = 0; suit < SUIT_COUNT; suit++) {
		for (int rank = LOWEST_RANK; rank <= ACE_RANK; rank++) {
			cards[index++] = Card(rank, static_cast<Suit>(suit));
		}
	}
}

Card Deck::drawCard() {
	if (topIndex < 0) {
		restack();
		shuffle();
	}
	return cards[topIndex--];
}

void Deck::shuffle() {
	std::shuffle(cards.begin(), cards.end(), generator);
}

void Deck::restack() {
	topIndex = SIZE - 1;
}

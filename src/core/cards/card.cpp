#include "card.hpp"
#include "core/common/text.hpp"

Card::Card(int rank, Suit suit) : rank(rank), suit(suit) {}

bool Card::isRed() const {
	return suit == Hearts || suit == Diamonds;
}

std::string Card::rankLabel() const {
	static const char* faceLabels[] = {"J", "Q", "K", "A"};
	if (rank >= JACK_RANK) return faceLabels[rank - JACK_RANK];
	return std::to_string(rank);
}

std::string Card::suitSymbol() const {
	static const char* symbols[] = {"♠", "♥", "♣", "♦"};
	return symbols[suit];
}

std::string Card::toString() const {
	return rankLabel() + suitSymbol();
}

std::vector<std::string> cardLabels(const Cards& cards) {
	std::vector<std::string> labels;
	for (const Card& card : cards) {
		labels.push_back(card.toString());
	}
	return labels;
}

std::string formatCards(const Cards& cards) {
	return join(cardLabels(cards), " ");
}

Cards combineCards(const Cards& first, const Cards& second) {
	Cards combined = first;
	combined.insert(combined.end(), second.begin(), second.end());
	return combined;
}

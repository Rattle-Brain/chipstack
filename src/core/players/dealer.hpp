#pragma once

#include "card_holder.hpp"
#include "core/cards/deck.hpp"

class Dealer : public CardHolder {
public:
	Dealer() : CardHolder("Dealer") {}
	Card drawCard() { return deck.drawCard(); }
	void dealTo(CardHolder& holder) { holder.receiveCard(drawCard()); }
	void shuffleDeck() { deck.shuffle(); }
	void restackDeck() { deck.restack(); }

private:
	Deck deck;
};

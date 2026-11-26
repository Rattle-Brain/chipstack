#include "crupier.hpp"

Crupier::Crupier() {
  name = "Crupier";
}

Crupier::~Crupier() {}

void Crupier::shuffleDeck() {
  deck.shuffle(deck.deck);
}

void Crupier::dealCard(IPlayer& player) {
  Card card = deck.pullCard();
  player.receiveCard(card);
}

Card Crupier::pullCard() {
  return deck.pullCard();
}

void Crupier::resetDeck() {
  deck.resetIndex();
}

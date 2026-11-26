#include "deck.hpp"
#include <random>

using namespace std;

Deck::Deck() {
  std::random_device rd;
  srand(rd());
  index = DECK_SIZE - 1;
  initDeck(Deck::deck);
}

Deck::~Deck() {}

Card Deck::pullCard(){
  return deck[index--];
}

void Deck::shuffle(Card deck[]) {
  srand(time(0));
  std::random_device rd; 
  std::mt19937 generator(rd()); 
  std::shuffle(deck, deck + DECK_SIZE, generator);
}

void Deck::resetIndex() {
  index = DECK_SIZE - 1;
}

int Deck::random(int n) {
  return rand() % (n + 1);
}
    
void Deck::initDeck(Card deck[]){
  int cardRank = MIN_CARD;
  int cardInt = (int)Spades;
  Suit cardSuit = static_cast<Suit>(cardInt);
  for (int i = 0; i < DECK_SIZE; i++) {
    deck[i] = Card(cardRank, cardSuit);
    cardRank++;
    if (cardRank > MAX_CARD){
      cardRank = MIN_CARD;
      cardInt++;
      cardSuit = static_cast<Suit>(cardInt);
    }
  }
}


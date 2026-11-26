#ifndef DECK_HPP
#define DECK_HPP

#include "card.hpp"
#include <cstddef>
#include <cstdlib>

#define DECK_SIZE 13*4

class Deck {
  public:
    Deck();
    virtual ~Deck();
    Card deck[DECK_SIZE];
    Card pullCard();
    void shuffle(Card[]);
    void resetIndex();
    
  private:
    void initDeck(Card[]);
    int random(int);
    int index;
};

#endif

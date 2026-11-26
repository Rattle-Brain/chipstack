#ifndef CRUPIER_HPP
#define CRUPIER_HPP

#include "iplayer.hpp"
#include "core/cards/deck.hpp"

class Crupier : public IPlayer {
  public:
    Crupier();
    ~Crupier();

    void shuffleDeck();
    void dealCard(IPlayer& player);
    Card pullCard();
    void resetDeck();

  private:
    Deck deck;
};

#endif

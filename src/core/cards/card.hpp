#ifndef CARD_HPP
#define CARD_HPP

#include <string>
#define MIN_CARD 2
#define MAX_CARD 14

typedef enum {
  Spades,
  Hearts,
  Clubs,
  Diamonds
}Suit;

class Card {
  public:
    // RAII
    Card();
    Card(int, Suit);
    ~Card();

    // Functions
    int getRank() const;
    Suit getSuit() const;
    std::string toString() const;

  private:
    // Variables
    int rank;
    Suit suit;
};

#endif

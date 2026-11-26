#include "card.hpp"

Card::Card(){
  Card::rank = 1;
  Card::suit = Suit::Spades;
}

Card::Card(int r, Suit s){
  Card::rank = r;
  Card::suit = s;
}

Card::~Card(){}

int Card::getRank() const { return rank; }

Suit Card::getSuit() const { return suit; }

std::string Card::toString() const {
  std::string suitChar;
  std::string cardValue;
    
  switch(getSuit()) {
    case Suit::Spades:
      suitChar = "♠";
      break;
    case Suit::Clubs:
      suitChar = "♣";
      break;
    case Suit::Hearts:
      suitChar = "♥";
      break;
    case Suit::Diamonds:
      suitChar = "♦";
      break;
  }
  if(getRank() == 14){
    cardValue = "A";
  }else if(getRank() == 13){
    cardValue = "K";
  }else if(getRank() == 12){
    cardValue = "Q";
  }else if(getRank() == 11){
    cardValue = "J";
  } else {
    cardValue = std::to_string(getRank());
  }

  return cardValue + suitChar;
}


#ifndef BETTING_CONTEXT_HPP
#define BETTING_CONTEXT_HPP

#include "core/cards/card.hpp"
#include <vector>

enum class BettingRoundType {
    PreFlop,
    Flop,
    Turn,
    River
};

enum class Position {
    EarlyBlind,
    Early,
    Middle,
    Late,
    Dealer
};

struct BettingContext {
    BettingRoundType roundType;
    std::vector<Card> communityCards;
    int pot;
    int currentBet;
    int playerCurrentBet;
    int costToCall;
    int playerChips;
    int numActivePlayers;
    int numPlayersToAct;
    int minRaise;
    int bigBlind;
    int dealerButton;
    int playerSeatIndex;
    int totalPlayers;
    Position position;
};

#endif

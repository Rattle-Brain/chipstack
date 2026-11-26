#ifndef BLACKJACK_ACTION_HPP
#define BLACKJACK_ACTION_HPP

enum class BlackjackActionType {
    Hit,
    Stand,
    DoubleDown
};

struct BlackjackAction {
    BlackjackActionType type;

    static BlackjackAction hit() { return {BlackjackActionType::Hit}; }
    static BlackjackAction stand() { return {BlackjackActionType::Stand}; }
    static BlackjackAction doubleDown() { return {BlackjackActionType::DoubleDown}; }

    const char* getActionName() const {
        switch (type) {
            case BlackjackActionType::Hit:        return "Hit";
            case BlackjackActionType::Stand:      return "Stand";
            case BlackjackActionType::DoubleDown: return "Double Down";
        }
        return "Unknown";
    }
};

struct BlackjackContext {
    int handValue;
    int numCards;
    bool hasSoftAce;
    int dealerUpcard;
    int bet;
    int playerChips;
    bool canDoubleDown;
};

struct BlackjackBetContext {
    int playerChips;
    int minBet;
    int maxBet;
};

#endif

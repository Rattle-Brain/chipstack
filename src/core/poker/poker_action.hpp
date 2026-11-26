#ifndef POKER_ACTION_HPP
#define POKER_ACTION_HPP

enum class ActionType {
    Fold,
    Check,
    Call,
    Raise,
    AllIn
};

struct PokerAction {
    ActionType type;
    int amount;

    static PokerAction fold() { return {ActionType::Fold, 0}; }
    static PokerAction check() { return {ActionType::Check, 0}; }
    static PokerAction call() { return {ActionType::Call, 0}; }
    static PokerAction raise(int amt) { return {ActionType::Raise, amt}; }
    static PokerAction allIn() { return {ActionType::AllIn, 0}; }

    const char* getActionName() const {
        switch (type) {
            case ActionType::Fold:  return "Fold";
            case ActionType::Check: return "Check";
            case ActionType::Call:  return "Call";
            case ActionType::Raise: return "Raise";
            case ActionType::AllIn: return "All-In";
        }
        return "Unknown";
    }
};

enum class PlayerHandState {
    Active,
    Folded,
    AllIn,
    SittingOut
};

#endif

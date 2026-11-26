#include "player.hpp"

Player::Player(std::string playerName, int initialChips) {
    name = playerName;
    chips = initialChips;
}

Player::~Player() {}

PokerAction Player::decideAction(const BettingContext& context) {
    if (delegate_) return delegate_->getPokerAction(context);
    return PokerAction::fold();
}

BlackjackAction Player::decideBlackjackPlay(const BlackjackContext& context) {
    if (delegate_) return delegate_->getBlackjackAction(context);
    return BlackjackAction::stand();
}

int Player::decideBlackjackBet(const BlackjackBetContext& context) {
    if (delegate_) return delegate_->getBlackjackBet(context);
    return context.minBet;
}

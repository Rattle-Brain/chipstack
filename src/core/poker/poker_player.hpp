#pragma once

#include "betting_context.hpp"
#include "core/players/player.hpp"
#include "poker_action.hpp"

class PokerPlayer : public virtual Player {
public:
	virtual PokerAction decidePokerAction(const BettingContext& context) = 0;
};

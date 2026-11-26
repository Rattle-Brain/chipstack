#pragma once

#include "blackjack_action.hpp"
#include "core/players/player.hpp"

class BlackjackPlayer : public virtual Player {
public:
	virtual int decideBlackjackBet(const BlackjackBetContext& context) = 0;
	virtual BlackjackAction decideBlackjackAction(
			const BlackjackContext& context
	) = 0;
};

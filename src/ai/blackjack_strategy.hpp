#pragma once

#include "bot_personality.hpp"
#include "core/blackjack/blackjack_action.hpp"
#include "random_source.hpp"

class BlackjackStrategy {
public:
	BlackjackStrategy(const BotPersonality& personality, RandomSource& random);

	int decideBet(const BlackjackBetContext& context);
	BlackjackAction decideAction(const BlackjackContext& context);

private:
	bool wantsDoubleDown(const BlackjackContext& context);
	bool standsOn(const BlackjackContext& context);
	bool noviceStands(int handValue);
	static bool goodStands(const BlackjackContext& context);
	static bool basicStrategyStands(const BlackjackContext& context);

	const BotPersonality& personality;
	RandomSource& random;
};

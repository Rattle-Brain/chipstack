#pragma once

#include "bot_personality.hpp"
#include "core/blackjack/blackjack_player.hpp"
#include "core/poker/poker_player.hpp"
#include "random_source.hpp"

class BotPlayer : public PokerPlayer, public BlackjackPlayer {
public:
	BotPlayer(std::string name, int chips, BotPersonality personality);

	PokerAction decidePokerAction(const BettingContext& context) override;
	int decideBlackjackBet(const BlackjackBetContext& context) override;
	BlackjackAction decideBlackjackAction(
			const BlackjackContext& context
	) override;

	const BotPersonality& getPersonality() const { return personality; }

private:
	BotPersonality personality;
	RandomSource random;
};

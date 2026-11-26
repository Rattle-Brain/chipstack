#include "bot_player.hpp"
#include "blackjack_strategy.hpp"
#include "poker_strategy.hpp"

BotPlayer::BotPlayer(std::string name, int chips, BotPersonality personality)
		: Player(std::move(name), chips),
			personality(normalizePersonality(personality)) {}

PokerAction BotPlayer::decidePokerAction(const BettingContext& context) {
	return PokerStrategy(personality, random, context, getHand()).decide();
}

int BotPlayer::decideBlackjackBet(const BlackjackBetContext& context) {
	return BlackjackStrategy(personality, random).decideBet(context);
}

BlackjackAction BotPlayer::decideBlackjackAction(
		const BlackjackContext& context
) {
	return BlackjackStrategy(personality, random).decideAction(context);
}

#pragma once

#include "bot_personality.hpp"
#include "core/poker/betting_context.hpp"
#include "core/poker/poker_action.hpp"
#include "random_source.hpp"
#include <optional>

struct PokerProfile;

class PokerStrategy {
public:
	PokerStrategy(
			const BotPersonality& personality, RandomSource& random,
			const BettingContext& context, const Cards& holeCards
	);

	PokerAction decide();

private:
	const PokerProfile& currentProfile() const;
	float perceivedStrength();
	float foldThreshold() const;
	PokerAction actNovice();
	PokerAction actGood(float perceived);
	PokerAction actGreat(float perceived);
	PokerAction actProfessional(float perceived);
	std::optional<PokerAction> trySlowPlay(float perceived);
	std::optional<PokerAction> tryCheckRaise(float perceived);
	std::optional<PokerAction> trySemiBluff(float perceived);

	float handStrength() const;
	float drawBonus() const;
	float impliedOddsBonus() const;
	float potOdds() const;
	float positionModifier() const;
	int raiseAmount(float strength) const;
	PokerAction raiseOrFallback(int amount, bool shoveWhenCapped) const;
	PokerAction checkOrCall() const;
	PokerAction checkOrFold() const;
	bool isInLatePosition() const;
	bool isInEarlyPosition() const;

	const BotPersonality& personality;
	RandomSource& random;
	const BettingContext& context;
	const Cards& holeCards;
};

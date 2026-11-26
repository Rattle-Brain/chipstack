#include "blackjack_strategy.hpp"
#include <algorithm>
#include <vector>

namespace {

struct DoubleDownRule {
	bool soft;
	int minTotal;
	int maxTotal;
	int minUpcard;
	int maxUpcard;
	float chance;
};

using DoubleDownRules = std::vector<DoubleDownRule>;

const DoubleDownRules DOUBLE_DOWN_RULES_BY_DIFFICULTY[] = {
		{},
		{
				{false, 11, 11, 2, 9, 0.80f},
				{false, 10, 10, 2, 9, 0.60f},
		},
		{
				{true, 16, 17, 4, 6, 0.90f},
				{true, 15, 15, 4, 6, 0.80f},
				{true, 13, 14, 5, 6, 0.70f},
				{false, 11, 11, 2, 11, 0.95f},
				{false, 10, 10, 2, 9, 0.90f},
				{false, 9, 9, 3, 6, 0.80f},
		},
		{
				{true, 17, 18, 3, 6, 1.0f},
				{true, 15, 16, 4, 6, 1.0f},
				{true, 13, 14, 5, 6, 1.0f},
				{false, 11, 11, 2, 11, 1.0f},
				{false, 10, 10, 2, 9, 1.0f},
				{false, 9, 9, 3, 6, 1.0f},
		},
};

const float BET_VARIANCE_BY_DIFFICULTY[] = {0.50f, 0.20f, 0.10f, 0.05f};

bool isBetween(int value, int min, int max) {
	return value >= min && value <= max;
}

bool ruleMatches(const DoubleDownRule& rule, const BlackjackContext& context) {
	return rule.soft == context.hasSoftAce
			&& isBetween(context.handValue, rule.minTotal, rule.maxTotal)
			&& isBetween(context.dealerUpcard, rule.minUpcard, rule.maxUpcard);
}

} // namespace

BlackjackStrategy::BlackjackStrategy(
		const BotPersonality& personality, RandomSource& random
)
		: personality(personality), random(random) {}

int BlackjackStrategy::decideBet(const BlackjackBetContext& context) {
	float variance =
			BET_VARIANCE_BY_DIFFICULTY[static_cast<int>(personality.difficulty)];
	float range = context.maxBet - context.minBet;
	float bet = context.minBet + range * personality.aggressiveness * 0.3f;
	bet *= 1.0f + random.uniform(-variance, variance);
	float risk = personality.riskTolerance;
	if (risk > 0.6f && random.chance(risk * 0.2f)) bet *= 1.0f + risk * 0.5f;
	int wager = std::max(static_cast<int>(bet), context.minBet);
	return std::min({wager, context.maxBet, context.playerChips});
}

BlackjackAction BlackjackStrategy::decideAction(
		const BlackjackContext& context
) {
	if (context.canDoubleDown && wantsDoubleDown(context)) {
		return BlackjackAction::doubleDown();
	}
	if (standsOn(context)) return BlackjackAction::stand();
	return BlackjackAction::hit();
}

bool BlackjackStrategy::wantsDoubleDown(const BlackjackContext& context) {
	const DoubleDownRules& rules =
			DOUBLE_DOWN_RULES_BY_DIFFICULTY[static_cast<int>(personality.difficulty)];
	for (const DoubleDownRule& rule : rules) {
		if (!ruleMatches(rule, context)) continue;
		if (rule.chance >= 1.0f) return true;
		return random.chance(rule.chance * personality.riskTolerance);
	}
	return false;
}

bool BlackjackStrategy::standsOn(const BlackjackContext& context) {
	switch (personality.difficulty) {
		case Difficulty::Novice:
			return noviceStands(context.handValue);
		case Difficulty::Good:
			return goodStands(context);
		default:
			return basicStrategyStands(context);
	}
}

bool BlackjackStrategy::noviceStands(int handValue) {
	int threshold =
			std::clamp(15 + static_cast<int>(random.uniform(-3.0f, 3.0f)), 12, 19);
	threshold -= static_cast<int>(personality.riskTolerance * 2.0f);
	return handValue >= threshold;
}

bool BlackjackStrategy::goodStands(const BlackjackContext& context) {
	if (context.handValue >= 17) return true;
	if (context.handValue < 12) return false;
	return isBetween(context.dealerUpcard, 2, 6);
}

bool BlackjackStrategy::basicStrategyStands(const BlackjackContext& context) {
	int total = context.handValue;
	int upcard = context.dealerUpcard;
	if (context.hasSoftAce) return total >= 19 || (total == 18 && upcard < 9);
	if (total >= 17) return true;
	if (total >= 13) return isBetween(upcard, 2, 6);
	return total == 12 && isBetween(upcard, 4, 6);
}

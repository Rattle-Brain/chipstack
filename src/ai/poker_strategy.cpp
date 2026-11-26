#include "poker_strategy.hpp"
#include "hand_strength.hpp"
#include <algorithm>

struct PokerProfile {
	float noise;
	float foldBase;
	float foldPerOpponent;
	float foldRiskWeight;
	bool usesPotOdds;
	bool usesPositionAndDraws;
	bool usesImpliedOdds;
};

namespace {

const PokerProfile PROFILE_BY_DIFFICULTY[] = {
		{0.25f, 0.40f, 0.00f, 0.20f, false, false, false},
		{0.10f, 0.30f, 0.00f, 0.15f, true, false, false},
		{0.03f, 0.30f, 0.03f, 0.15f, true, true, false},
		{0.00f, 0.28f, 0.04f, 0.12f, true, true, true},
};

} // namespace

PokerStrategy::PokerStrategy(
		const BotPersonality& personality, RandomSource& random,
		const BettingContext& context, const Cards& holeCards
)
		: personality(personality), random(random), context(context),
			holeCards(holeCards) {}

const PokerProfile& PokerStrategy::currentProfile() const {
	return PROFILE_BY_DIFFICULTY[static_cast<int>(personality.difficulty)];
}

PokerAction PokerStrategy::decide() {
	float perceived = perceivedStrength();
	if (perceived < foldThreshold()) return checkOrFold();
	switch (personality.difficulty) {
		case Difficulty::Novice:
			return actNovice();
		case Difficulty::Good:
			return actGood(perceived);
		case Difficulty::Great:
			return actGreat(perceived);
		case Difficulty::Professional:
			return actProfessional(perceived);
	}
	return checkOrFold();
}

float PokerStrategy::perceivedStrength() {
	const PokerProfile& profile = currentProfile();
	float perceived = handStrength();
	if (profile.usesPositionAndDraws) {
		perceived += positionModifier() + drawBonus();
	}
	if (profile.usesImpliedOdds) perceived += impliedOddsBonus();
	if (profile.noise > 0)
		perceived += random.uniform(-profile.noise, profile.noise);
	return perceived;
}

float PokerStrategy::foldThreshold() const {
	const PokerProfile& profile = currentProfile();
	int extraOpponents = context.numActivePlayers - 2;
	float threshold = profile.foldBase + extraOpponents * profile.foldPerOpponent
			- personality.riskTolerance * profile.foldRiskWeight;
	return profile.usesPotOdds ? std::max(potOdds(), threshold) : threshold;
}

PokerAction PokerStrategy::actNovice() {
	if (!random.chance(personality.aggressiveness * 0.5f)) return checkOrCall();
	int amount =
			context.bigBlind * static_cast<int>(1 + random.uniform(1.0f, 3.0f));
	if (context.currentBet > 0) {
		int extra =
				context.bigBlind * static_cast<int>(1 + random.uniform(0.5f, 2.0f));
		amount = context.currentBet + extra;
	}
	return raiseOrFallback(amount, true);
}

PokerAction PokerStrategy::actGood(float perceived) {
	if (perceived <= 0.65f - personality.aggressiveness * 0.15f) {
		return checkOrCall();
	}
	bool shove =
			perceived > 0.80f || random.chance(personality.riskTolerance * 0.3f);
	return raiseOrFallback(raiseAmount(perceived), shove);
}

PokerAction PokerStrategy::actGreat(float perceived) {
	if (perceived > 0.80f) {
		return raiseOrFallback(
				static_cast<int>(raiseAmount(perceived) * 1.5f), true
		);
	}
	if (perceived <= 0.60f - personality.aggressiveness * 0.15f) {
		return checkOrCall();
	}
	bool shove =
			perceived > 0.75f || random.chance(personality.riskTolerance * 0.4f);
	return raiseOrFallback(raiseAmount(perceived), shove);
}

PokerAction PokerStrategy::actProfessional(float perceived) {
	if (auto action = trySlowPlay(perceived)) return *action;
	if (auto action = tryCheckRaise(perceived)) return *action;
	if (auto action = trySemiBluff(perceived)) return *action;
	if (perceived <= 0.55f - personality.aggressiveness * 0.12f) {
		return checkOrCall();
	}
	int amount =
			static_cast<int>(raiseAmount(perceived) * random.uniform(0.85f, 1.15f));
	bool shove = perceived > 0.70f || personality.riskTolerance > 0.7f;
	return raiseOrFallback(amount, shove);
}

std::optional<PokerAction> PokerStrategy::trySlowPlay(float perceived) {
	if (perceived <= 0.90f || !isInLatePosition()) return std::nullopt;
	float slowPlayChance = 0.30f + (1.0f - personality.aggressiveness) * 0.20f;
	if (!random.chance(slowPlayChance)) return std::nullopt;
	return checkOrCall();
}

std::optional<PokerAction> PokerStrategy::tryCheckRaise(float perceived) {
	bool strong = perceived > 0.75f && perceived < 0.90f;
	bool canCheck = context.costToCall == 0;
	if (!strong || !canCheck || !isInEarlyPosition()) return std::nullopt;
	if (!random.chance(0.25f + personality.aggressiveness * 0.15f)) {
		return std::nullopt;
	}
	return PokerAction::check();
}

std::optional<PokerAction> PokerStrategy::trySemiBluff(float perceived) {
	if (drawBonus() <= 0.15f) return std::nullopt;
	if (!random.chance(0.40f + personality.aggressiveness * 0.20f)) {
		return std::nullopt;
	}
	bool shove = personality.riskTolerance > 0.6f;
	return raiseOrFallback(raiseAmount(perceived), shove);
}

float PokerStrategy::handStrength() const {
	if (holeCards.size() < 2) return 0.0f;
	if (context.roundType != BettingRoundType::PreFlop) {
		return HandStrength::postflopStrength(holeCards, context.communityCards);
	}
	bool suited = holeCards[0].getSuit() == holeCards[1].getSuit();
	return HandStrength::preflopStrength(
			holeCards[0].getRank(), holeCards[1].getRank(), suited
	);
}

float PokerStrategy::drawBonus() const {
	bool drawsPossible = context.roundType == BettingRoundType::Flop
			|| context.roundType == BettingRoundType::Turn;
	if (!drawsPossible) return 0.0f;
	int outs = HandStrength::countDrawOuts(holeCards, context.communityCards);
	return outs / 46.0f;
}

float PokerStrategy::impliedOddsBonus() const {
	bool deepStacked = context.playerChips > context.pot * 2;
	return deepStacked ? drawBonus() * 0.5f : 0.0f;
}

float PokerStrategy::potOdds() const {
	if (context.costToCall <= 0) return 0.0f;
	return static_cast<float>(context.costToCall)
			/ (context.pot + context.costToCall);
}

float PokerStrategy::positionModifier() const {
	static const float modifiers[] = {-0.04f, -0.06f, 0.0f, 0.06f, 0.08f};
	return modifiers[static_cast<int>(context.position)];
}

int PokerStrategy::raiseAmount(float strength) const {
	float multiplier = 2.0f + personality.aggressiveness * 2.0f;
	int amount = std::max(
			static_cast<int>(context.currentBet * multiplier), context.bigBlind * 2
	);
	amount += static_cast<int>((strength - 0.5f) * context.currentBet);
	amount = std::max(amount, context.minRaise + context.currentBet);
	return std::min(amount, context.playerChips + context.playerCurrentBet);
}

PokerAction PokerStrategy::raiseOrFallback(
		int amount, bool shoveWhenCapped
) const {
	bool capped = amount >= context.playerChips + context.playerCurrentBet;
	if (!capped) return PokerAction::raise(amount);
	return shoveWhenCapped ? PokerAction::allIn() : PokerAction::call();
}

PokerAction PokerStrategy::checkOrCall() const {
	return context.costToCall == 0 ? PokerAction::check() : PokerAction::call();
}

PokerAction PokerStrategy::checkOrFold() const {
	return context.costToCall == 0 ? PokerAction::check() : PokerAction::fold();
}

bool PokerStrategy::isInLatePosition() const {
	return context.position == Position::Late
			|| context.position == Position::Dealer;
}

bool PokerStrategy::isInEarlyPosition() const {
	return context.position == Position::Early
			|| context.position == Position::EarlyBlind;
}

#include "bot_player.hpp"
#include <algorithm>

BotPlayer::BotPlayer(std::string botName, int initialChips,
                     float riskTolerance, float aggressiveness, Difficulty difficulty)
    : riskTolerance(riskTolerance), aggressiveness(aggressiveness),
      difficulty(difficulty), rng(std::random_device{}()) {
    name = botName;
    chips = initialChips;
    validateParameters();
}

BotPlayer::~BotPlayer() {}

void BotPlayer::validateParameters() {
    riskTolerance = std::max(0.0f, std::min(1.0f, riskTolerance));
    aggressiveness = std::max(0.0f, std::min(1.0f, aggressiveness));

    float maxAggressiveness = riskTolerance + 0.15f;
    if (aggressiveness > maxAggressiveness) {
        aggressiveness = maxAggressiveness;
    }
}

float BotPlayer::getRiskTolerance() const { return riskTolerance; }
float BotPlayer::getAggressiveness() const { return aggressiveness; }
Difficulty BotPlayer::getDifficulty() const { return difficulty; }

const char* BotPlayer::getDifficultyName() const {
    switch (difficulty) {
        case Difficulty::Novice:       return "Novice";
        case Difficulty::Good:         return "Good";
        case Difficulty::Great:        return "Great";
        case Difficulty::Professional: return "Professional";
    }
    return "Unknown";
}

float BotPlayer::randomFloat(float min, float max) const {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

float BotPlayer::evaluateHandStrength(const BettingContext& context) const {
    const std::vector<Card>& holeCards = getHand();
    if (holeCards.size() < 2) return 0.0f;

    if (context.roundType == BettingRoundType::PreFlop) {
        bool suited = (holeCards[0].getSuit() == holeCards[1].getSuit());
        return HandStrength::preflopStrength(
            holeCards[0].getRank(), holeCards[1].getRank(), suited);
    }

    return HandStrength::postflopStrength(holeCards, context.communityCards);
}

float BotPlayer::computePotOdds(const BettingContext& context) const {
    if (context.costToCall <= 0) return 0.0f;
    return (float)context.costToCall / (float)(context.pot + context.costToCall);
}

float BotPlayer::computePositionModifier(const BettingContext& context) const {
    switch (context.position) {
        case Position::Dealer:     return 0.08f;
        case Position::Late:       return 0.06f;
        case Position::Middle:     return 0.0f;
        case Position::Early:      return -0.06f;
        case Position::EarlyBlind: return -0.04f;
    }
    return 0.0f;
}

int BotPlayer::computeRaiseAmount(const BettingContext& context, float handStrength) const {
    float multiplier = 2.0f + aggressiveness * 2.0f;
    int base = (int)(context.currentBet * multiplier);

    if (base < context.bigBlind * 2) {
        base = context.bigBlind * 2;
    }

    float strengthBonus = (handStrength - 0.5f) * context.currentBet;
    base += (int)strengthBonus;

    if (base < context.minRaise + context.currentBet) {
        base = context.minRaise + context.currentBet;
    }
    if (base > context.playerChips + context.playerCurrentBet) {
        base = context.playerChips + context.playerCurrentBet;
    }

    return base;
}

PokerAction BotPlayer::makeFoldOrCheck(const BettingContext& context) const {
    if (context.costToCall == 0) return PokerAction::check();
    return PokerAction::fold();
}

PokerAction BotPlayer::decideAction(const BettingContext& context) {
    switch (difficulty) {
        case Difficulty::Novice:       return decideNovice(context);
        case Difficulty::Good:         return decideGood(context);
        case Difficulty::Great:        return decideGreat(context);
        case Difficulty::Professional: return decideProfessional(context);
    }
    return PokerAction::fold();
}

// --- NOVICE ---
// Large random noise, no pot odds, no position awareness.
// Plays basically by feel — sometimes folds good hands, sometimes calls with trash.
PokerAction BotPlayer::decideNovice(const BettingContext& context) const {
    float handStrength = evaluateHandStrength(context);
    float noise = randomFloat(-0.25f, 0.25f);
    float perceived = handStrength + noise;

    float foldThreshold = 0.40f - (riskTolerance * 0.20f);

    if (perceived < foldThreshold) {
        return makeFoldOrCheck(context);
    }

    // Random chance to raise based on aggressiveness
    if (randomFloat(0.0f, 1.0f) < aggressiveness * 0.5f) {
        int raiseAmount = context.bigBlind * (int)(1 + randomFloat(1.0f, 3.0f));
        if (context.currentBet > 0) {
            raiseAmount = context.currentBet + context.bigBlind * (int)(1 + randomFloat(0.5f, 2.0f));
        }
        if (raiseAmount >= context.playerChips + context.playerCurrentBet) {
            return PokerAction::allIn();
        }
        return PokerAction::raise(raiseAmount);
    }

    if (context.costToCall == 0) return PokerAction::check();
    return PokerAction::call();
}

// --- GOOD ---
// Considers pot odds. Won't call huge bets with weak hands.
// Basic raise logic scaled by aggressiveness.
PokerAction BotPlayer::decideGood(const BettingContext& context) const {
    float handStrength = evaluateHandStrength(context);
    float potOdds = computePotOdds(context);
    float noise = randomFloat(-0.10f, 0.10f);
    float perceived = handStrength + noise;

    float foldThreshold = std::max(potOdds, 0.30f - riskTolerance * 0.15f);

    if (perceived < foldThreshold) {
        return makeFoldOrCheck(context);
    }

    float raiseThreshold = 0.65f - (aggressiveness * 0.15f);
    if (perceived > raiseThreshold) {
        int raiseAmount = computeRaiseAmount(context, perceived);
        if (raiseAmount >= context.playerChips + context.playerCurrentBet) {
            // Only go all-in with strong hands or high risk tolerance
            if (perceived > 0.80f || randomFloat(0.0f, 1.0f) < riskTolerance * 0.3f) {
                return PokerAction::allIn();
            }
            return PokerAction::call();
        }
        return PokerAction::raise(raiseAmount);
    }

    if (context.costToCall == 0) return PokerAction::check();
    return PokerAction::call();
}

// --- GREAT ---
// Position-aware, counts draw outs, adjusts for number of opponents.
PokerAction BotPlayer::decideGreat(const BettingContext& context) const {
    float handStrength = evaluateHandStrength(context);
    float potOdds = computePotOdds(context);
    float positionMod = computePositionModifier(context);
    float noise = randomFloat(-0.03f, 0.03f);

    float drawBonus = 0.0f;
    if (context.roundType != BettingRoundType::PreFlop &&
        context.roundType != BettingRoundType::River) {
        int outs = HandStrength::countDrawOuts(getHand(), context.communityCards);
        drawBonus = (float)outs / 46.0f;
    }

    float perceived = handStrength + positionMod + drawBonus + noise;

    float playerMod = (context.numActivePlayers - 2) * 0.03f;
    float foldThreshold = std::max(potOdds, 0.30f + playerMod - riskTolerance * 0.15f);

    if (perceived < foldThreshold) {
        return makeFoldOrCheck(context);
    }

    float raiseThreshold = 0.60f - (aggressiveness * 0.15f);
    float bigRaiseThreshold = 0.80f;

    if (perceived > bigRaiseThreshold) {
        int raiseAmount = computeRaiseAmount(context, perceived);
        // Scale up for big hands
        raiseAmount = (int)(raiseAmount * 1.5f);
        if (raiseAmount >= context.playerChips + context.playerCurrentBet) {
            return PokerAction::allIn();
        }
        return PokerAction::raise(raiseAmount);
    }

    if (perceived > raiseThreshold) {
        int raiseAmount = computeRaiseAmount(context, perceived);
        if (raiseAmount >= context.playerChips + context.playerCurrentBet) {
            if (perceived > 0.75f || randomFloat(0.0f, 1.0f) < riskTolerance * 0.4f) {
                return PokerAction::allIn();
            }
            return PokerAction::call();
        }
        return PokerAction::raise(raiseAmount);
    }

    if (context.costToCall == 0) return PokerAction::check();
    return PokerAction::call();
}

// --- PROFESSIONAL ---
// No noise. Implied odds for draws. Can slow-play/trap strong hands.
// Optimal bet sizing. Hardest to read.
PokerAction BotPlayer::decideProfessional(const BettingContext& context) const {
    float handStrength = evaluateHandStrength(context);
    float potOdds = computePotOdds(context);
    float positionMod = computePositionModifier(context);

    float drawBonus = 0.0f;
    float impliedOddsAdj = 0.0f;
    if (context.roundType != BettingRoundType::PreFlop &&
        context.roundType != BettingRoundType::River) {
        int outs = HandStrength::countDrawOuts(getHand(), context.communityCards);
        drawBonus = (float)outs / 46.0f;

        // Implied odds: if we have chips behind and a draw, the potential payout is bigger
        if (drawBonus > 0 && context.playerChips > context.pot * 2) {
            impliedOddsAdj = drawBonus * 0.5f;
        }
    }

    float perceived = handStrength + positionMod + drawBonus + impliedOddsAdj;

    float playerMod = (context.numActivePlayers - 2) * 0.04f;
    float foldThreshold = std::max(potOdds, 0.28f + playerMod - riskTolerance * 0.12f);

    if (perceived < foldThreshold) {
        return makeFoldOrCheck(context);
    }

    // Trap / slow-play: with monster hands in position, disguise strength
    if (perceived > 0.90f &&
        (context.position == Position::Late || context.position == Position::Dealer)) {
        float trapChance = 0.30f + (1.0f - aggressiveness) * 0.20f;
        if (randomFloat(0.0f, 1.0f) < trapChance) {
            if (context.costToCall == 0) return PokerAction::check();
            return PokerAction::call();
        }
    }

    // Check-raise potential: strong hand in early position, check to induce a bet
    if (perceived > 0.75f && perceived < 0.90f &&
        context.costToCall == 0 &&
        (context.position == Position::Early || context.position == Position::EarlyBlind)) {
        float checkRaiseChance = 0.25f + aggressiveness * 0.15f;
        if (randomFloat(0.0f, 1.0f) < checkRaiseChance) {
            return PokerAction::check();
        }
    }

    // Semi-bluff draws aggressively
    if (drawBonus > 0.15f && context.roundType != BettingRoundType::River) {
        float semiBluffChance = 0.40f + aggressiveness * 0.20f;
        if (randomFloat(0.0f, 1.0f) < semiBluffChance) {
            int raiseAmount = computeRaiseAmount(context, perceived);
            if (raiseAmount >= context.playerChips + context.playerCurrentBet) {
                if (riskTolerance > 0.6f) return PokerAction::allIn();
                return PokerAction::call();
            }
            return PokerAction::raise(raiseAmount);
        }
    }

    float raiseThreshold = 0.55f - (aggressiveness * 0.12f);

    if (perceived > raiseThreshold) {
        int raiseAmount = computeRaiseAmount(context, perceived);

        // Vary sizing to be less predictable
        float sizingVariation = randomFloat(0.85f, 1.15f);
        raiseAmount = (int)(raiseAmount * sizingVariation);

        if (raiseAmount >= context.playerChips + context.playerCurrentBet) {
            if (perceived > 0.70f || riskTolerance > 0.7f) {
                return PokerAction::allIn();
            }
            return PokerAction::call();
        }
        return PokerAction::raise(raiseAmount);
    }

    if (context.costToCall == 0) return PokerAction::check();
    return PokerAction::call();
}

// =============================================================================
// Blackjack AI
// =============================================================================

int BotPlayer::decideBlackjackBet(const BlackjackBetContext& context) {
    return computeBjBetAmount(context);
}

int BotPlayer::computeBjBetAmount(const BlackjackBetContext& context) const {
    float range = (float)(context.maxBet - context.minBet);
    float baseBet = context.minBet + range * aggressiveness * 0.3f;

    float variance = 0.0f;
    switch (difficulty) {
        case Difficulty::Novice:       variance = randomFloat(-0.50f, 0.50f); break;
        case Difficulty::Good:         variance = randomFloat(-0.20f, 0.20f); break;
        case Difficulty::Great:        variance = randomFloat(-0.10f, 0.10f); break;
        case Difficulty::Professional: variance = randomFloat(-0.05f, 0.05f); break;
    }

    baseBet *= (1.0f + variance);

    // High risk tolerance occasionally makes bigger bets
    if (riskTolerance > 0.6f && randomFloat(0.0f, 1.0f) < riskTolerance * 0.2f) {
        baseBet *= (1.0f + riskTolerance * 0.5f);
    }

    int bet = (int)baseBet;
    if (bet < context.minBet) bet = context.minBet;
    if (bet > context.maxBet) bet = context.maxBet;
    if (bet > context.playerChips) bet = context.playerChips;
    return bet;
}

BlackjackAction BotPlayer::decideBlackjackPlay(const BlackjackContext& context) {
    switch (difficulty) {
        case Difficulty::Novice:       return bjDecideNovice(context);
        case Difficulty::Good:         return bjDecideGood(context);
        case Difficulty::Great:        return bjDecideGreat(context);
        case Difficulty::Professional: return bjDecideProfessional(context);
    }
    return BlackjackAction::stand();
}

bool BotPlayer::shouldDoubleDown(const BlackjackContext& context, float threshold) const {
    if (!context.canDoubleDown) return false;
    return randomFloat(0.0f, 1.0f) < threshold * riskTolerance;
}

// --- NOVICE BLACKJACK ---
// Random stand threshold, ignores dealer card, never doubles down.
BlackjackAction BotPlayer::bjDecideNovice(const BlackjackContext& context) const {
    int noise = (int)randomFloat(-3.0f, 3.0f);
    int standThreshold = 15 + noise;

    if (standThreshold < 12) standThreshold = 12;
    if (standThreshold > 19) standThreshold = 19;

    standThreshold -= (int)(riskTolerance * 2.0f);

    if (context.handValue >= standThreshold) {
        return BlackjackAction::stand();
    }
    return BlackjackAction::hit();
}

// --- GOOD BLACKJACK ---
// Basic hit/stand: hard 17+ stand, hit below 12.
// 12-16: stand vs dealer 2-6, hit vs 7+.
// Doubles on 10-11 vs dealer 2-9.
BlackjackAction BotPlayer::bjDecideGood(const BlackjackContext& context) const {
    int value = context.handValue;
    int dealer = context.dealerUpcard;
    int noise = (int)randomFloat(-1.0f, 1.0f);
    int adjustedValue = value + noise;

    if (context.canDoubleDown) {
        if (value == 11 && dealer >= 2 && dealer <= 9) {
            if (shouldDoubleDown(context, 0.8f)) return BlackjackAction::doubleDown();
        }
        if (value == 10 && dealer >= 2 && dealer <= 9) {
            if (shouldDoubleDown(context, 0.6f)) return BlackjackAction::doubleDown();
        }
    }

    if (adjustedValue >= 17) return BlackjackAction::stand();
    if (adjustedValue < 12) return BlackjackAction::hit();

    bool dealerWeak = (dealer >= 2 && dealer <= 6);
    if (dealerWeak) return BlackjackAction::stand();
    return BlackjackAction::hit();
}

// --- GREAT BLACKJACK ---
// Near-perfect basic strategy. Handles soft hands.
BlackjackAction BotPlayer::bjDecideGreat(const BlackjackContext& context) const {
    int value = context.handValue;
    int dealer = context.dealerUpcard;
    bool soft = context.hasSoftAce;

    float noise = randomFloat(-0.5f, 0.5f);
    int adjustedValue = value + (int)noise;

    if (soft) {
        if (context.canDoubleDown) {
            if (value >= 16 && value <= 17 && dealer >= 4 && dealer <= 6) {
                if (shouldDoubleDown(context, 0.9f)) return BlackjackAction::doubleDown();
            }
            if (value == 15 && dealer >= 4 && dealer <= 6) {
                if (shouldDoubleDown(context, 0.8f)) return BlackjackAction::doubleDown();
            }
            if (value >= 13 && value <= 14 && dealer >= 5 && dealer <= 6) {
                if (shouldDoubleDown(context, 0.7f)) return BlackjackAction::doubleDown();
            }
        }

        if (value >= 19) return BlackjackAction::stand();
        if (value == 18) {
            if (dealer >= 9) return BlackjackAction::hit();
            return BlackjackAction::stand();
        }
        return BlackjackAction::hit();
    }

    if (context.canDoubleDown) {
        if (value == 11) {
            if (shouldDoubleDown(context, 0.95f)) return BlackjackAction::doubleDown();
        }
        if (value == 10 && dealer >= 2 && dealer <= 9) {
            if (shouldDoubleDown(context, 0.9f)) return BlackjackAction::doubleDown();
        }
        if (value == 9 && dealer >= 3 && dealer <= 6) {
            if (shouldDoubleDown(context, 0.8f)) return BlackjackAction::doubleDown();
        }
    }

    if (adjustedValue >= 17) return BlackjackAction::stand();

    if (adjustedValue >= 13) {
        if (dealer >= 2 && dealer <= 6) return BlackjackAction::stand();
        return BlackjackAction::hit();
    }

    if (adjustedValue == 12) {
        if (dealer >= 4 && dealer <= 6) return BlackjackAction::stand();
        return BlackjackAction::hit();
    }

    return BlackjackAction::hit();
}

// --- PROFESSIONAL BLACKJACK ---
// Perfect basic strategy. No noise. Always doubles when correct.
BlackjackAction BotPlayer::bjDecideProfessional(const BlackjackContext& context) const {
    int value = context.handValue;
    int dealer = context.dealerUpcard;
    bool soft = context.hasSoftAce;

    if (soft) {
        if (context.canDoubleDown) {
            if (value == 18 && dealer >= 3 && dealer <= 6) return BlackjackAction::doubleDown();
            if (value == 17 && dealer >= 3 && dealer <= 6) return BlackjackAction::doubleDown();
            if (value == 16 && dealer >= 4 && dealer <= 6) return BlackjackAction::doubleDown();
            if (value == 15 && dealer >= 4 && dealer <= 6) return BlackjackAction::doubleDown();
            if (value == 14 && dealer >= 5 && dealer <= 6) return BlackjackAction::doubleDown();
            if (value == 13 && dealer >= 5 && dealer <= 6) return BlackjackAction::doubleDown();
        }

        if (value >= 20) return BlackjackAction::stand();
        if (value == 19) return BlackjackAction::stand();
        if (value == 18) {
            if (dealer >= 9) return BlackjackAction::hit();
            return BlackjackAction::stand();
        }
        return BlackjackAction::hit();
    }

    if (context.canDoubleDown) {
        if (value == 11) return BlackjackAction::doubleDown();
        if (value == 10 && dealer >= 2 && dealer <= 9) return BlackjackAction::doubleDown();
        if (value == 9 && dealer >= 3 && dealer <= 6) return BlackjackAction::doubleDown();
    }

    if (value >= 17) return BlackjackAction::stand();

    if (value >= 13 && value <= 16) {
        if (dealer >= 2 && dealer <= 6) return BlackjackAction::stand();
        return BlackjackAction::hit();
    }

    if (value == 12) {
        if (dealer >= 4 && dealer <= 6) return BlackjackAction::stand();
        return BlackjackAction::hit();
    }

    return BlackjackAction::hit();
}

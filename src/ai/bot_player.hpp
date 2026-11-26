#ifndef BOT_PLAYER_HPP
#define BOT_PLAYER_HPP

#include "core/common/iplayer.hpp"
#include "core/poker/poker_action.hpp"
#include "core/poker/betting_context.hpp"
#include "core/blackjack/blackjack_action.hpp"
#include "hand_strength.hpp"
#include <random>

enum class Difficulty {
    Novice,
    Good,
    Great,
    Professional
};

class BotPlayer : public IPlayer {
  public:
    BotPlayer(std::string name, int initialChips,
              float riskTolerance, float aggressiveness, Difficulty difficulty);
    ~BotPlayer();

    PokerAction decideAction(const BettingContext& context) override;
    BlackjackAction decideBlackjackPlay(const BlackjackContext& context) override;
    int decideBlackjackBet(const BlackjackBetContext& context) override;

    float getRiskTolerance() const;
    float getAggressiveness() const;
    Difficulty getDifficulty() const;
    const char* getDifficultyName() const;

  private:
    float riskTolerance;
    float aggressiveness;
    Difficulty difficulty;
    mutable std::mt19937 rng;

    void validateParameters();

    float evaluateHandStrength(const BettingContext& context) const;
    float computePotOdds(const BettingContext& context) const;
    float computePositionModifier(const BettingContext& context) const;
    int computeRaiseAmount(const BettingContext& context, float handStrength) const;
    float randomFloat(float min, float max) const;

    PokerAction decideNovice(const BettingContext& context) const;
    PokerAction decideGood(const BettingContext& context) const;
    PokerAction decideGreat(const BettingContext& context) const;
    PokerAction decideProfessional(const BettingContext& context) const;
    PokerAction makeFoldOrCheck(const BettingContext& context) const;

    // Blackjack AI
    BlackjackAction bjDecideNovice(const BlackjackContext& context) const;
    BlackjackAction bjDecideGood(const BlackjackContext& context) const;
    BlackjackAction bjDecideGreat(const BlackjackContext& context) const;
    BlackjackAction bjDecideProfessional(const BlackjackContext& context) const;
    int computeBjBetAmount(const BlackjackBetContext& context) const;
    bool shouldDoubleDown(const BlackjackContext& context, float threshold) const;
};

#endif

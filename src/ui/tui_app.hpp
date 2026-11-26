#ifndef TUI_APP_HPP
#define TUI_APP_HPP

#include "ai/bot_player.hpp"
#include <string>
#include <vector>

struct BotConfig {
    std::string name;
    Difficulty difficulty;
    float riskTolerance;
    float aggressiveness;
};

struct GameConfig {
    enum GameType { Poker, Blackjack };
    GameType gameType = Poker;

    std::string playerName = "You";
    int startingChips = 1000;

    std::vector<BotConfig> bots;

    int smallBlind = 10;
    int bigBlind = 20;

    int minBet = 10;
    int maxBet = 500;

    int handLimit = 0; // 0 = unlimited
};

class TuiApp {
  public:
    void run();

  private:
    GameConfig config;

    bool showMainMenu();
    bool showPokerScenarios();
    bool showBlackjackScenarios();
    bool showCustomPokerConfig();
    bool showCustomBlackjackConfig();
    bool showChipSelect();
    bool runGame();

    bool runPokerGame();
    bool runBlackjackGame();

    static std::vector<BotConfig> makePresetBots(const std::vector<std::pair<std::string, Difficulty>>& specs);
};

#endif

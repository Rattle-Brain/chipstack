#ifndef GAME_STATE_HPP
#define GAME_STATE_HPP

#include <mutex>
#include <condition_variable>
#include <vector>
#include <string>
#include <optional>
#include "core/poker/poker_action.hpp"
#include "core/poker/betting_context.hpp"
#include "core/blackjack/blackjack_action.hpp"

struct PlayerDisplayInfo {
    std::string name;
    int chips = 0;
    std::vector<std::string> cards;
    bool showCards = false;
    int currentBet = 0;
    std::string lastAction;
    bool isActive = true;
    bool isFolded = false;
    bool isAllIn = false;
    bool isCurrentTurn = false;
    bool isHuman = false;
    bool isDealer = false;
    bool isThinking = false;
};

struct PokerGameState {
    std::mutex mutex;

    std::vector<PlayerDisplayInfo> players;
    std::vector<std::string> communityCards;
    int pot = 0;
    std::string phaseName;
    int handNumber = 0;
    int smallBlind = 0;
    int bigBlind = 0;

    std::vector<std::string> actionLog;
    static constexpr int MAX_LOG = 10;

    std::string dealerAnnouncement;

    bool waitingForHumanAction = false;
    BettingContext humanContext{};
    std::optional<PokerAction> humanResponse;
    std::condition_variable humanInputCV;

    bool handComplete = false;
    bool gameOver = false;
    std::string gameOverMessage;
    bool waitingForNextHand = false;
    bool quitRequested = false;

    bool paused = false;
    int pauseSelection = 0;
    bool exitApp = false;

    void addLogEntry(const std::string& entry) {
        actionLog.push_back(entry);
        if ((int)actionLog.size() > MAX_LOG)
            actionLog.erase(actionLog.begin());
    }
};

struct BlackjackGameState {
    std::mutex mutex;

    std::vector<PlayerDisplayInfo> players;
    PlayerDisplayInfo dealer;
    bool showDealerHoleCard = false;
    int handNumber = 0;
    int minBet = 0;
    int maxBet = 0;

    std::vector<std::string> actionLog;
    static constexpr int MAX_LOG = 10;

    std::string dealerAnnouncement;

    bool waitingForHumanBjAction = false;
    BlackjackContext humanBjContext{};
    std::optional<BlackjackAction> humanBjResponse;
    std::condition_variable humanBjInputCV;

    bool waitingForHumanBet = false;
    BlackjackBetContext humanBetContext{};
    std::optional<int> humanBetResponse;
    std::condition_variable humanBetInputCV;

    bool handComplete = false;
    bool gameOver = false;
    std::string gameOverMessage;
    bool waitingForNextHand = false;
    bool quitRequested = false;

    bool paused = false;
    int pauseSelection = 0;
    bool exitApp = false;

    void addLogEntry(const std::string& entry) {
        actionLog.push_back(entry);
        if ((int)actionLog.size() > MAX_LOG)
            actionLog.erase(actionLog.begin());
    }
};

#endif

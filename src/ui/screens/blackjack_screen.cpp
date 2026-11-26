#include "blackjack_screen.hpp"
#include "ui/tui_app.hpp"
#include "ui/game_state.hpp"
#include "ui/card_renderer.hpp"
#include "ui/layout/table_layout.hpp"
#include "core/blackjack/blackjack_table.hpp"
#include "core/common/player.hpp"
#include "core/common/decision_delegate.hpp"
#include "core/common/game_observer.hpp"
#include "ai/bot_player.hpp"
#include "ai/thinking_delay.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <thread>
#include <sstream>

using namespace ftxui;

// ============================================================================
// Blackjack TUI Observer
// ============================================================================

class BlackjackTuiObserver : public IGameObserver {
    BlackjackGameState& state;
    ScreenInteractive& screen;
public:
    BlackjackTuiObserver(BlackjackGameState& s, ScreenInteractive& scr) : state(s), screen(scr) {}

    void onBetPlaced(const std::string& player, int amount) override {
        std::lock_guard<std::mutex> lock(state.mutex);
        for (PlayerDisplayInfo& p : state.players) {
            if (p.name == player) { p.currentBet = amount; break; }
        }
        state.addLogEntry(player + " bets " + std::to_string(amount));
        screen.Post(Event::Custom);
    }

    void onDealingStarted() override {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.addLogEntry("Dealing...");
        screen.Post(Event::Custom);
    }

    void onPlayerDealt(const std::string& player, const std::string& cards, int value) override {
        std::lock_guard<std::mutex> lock(state.mutex);
        for (PlayerDisplayInfo& p : state.players) {
            if (p.name == player) {
                p.cards.clear();
                std::istringstream iss(cards);
                std::string card;
                while (iss >> card) p.cards.push_back(card);
                p.lastAction = "Value: " + std::to_string(value);
                break;
            }
        }
        screen.Post(Event::Custom);
    }

    void onDealerDealt(const std::string& card) override {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.dealer.cards = {card, "??"};
        state.dealer.lastAction = "Showing: " + card;
        screen.Post(Event::Custom);
    }

    void onPlayerHit(const std::string& player, const std::string& cards, int value) override {
        std::lock_guard<std::mutex> lock(state.mutex);
        for (PlayerDisplayInfo& p : state.players) {
            if (p.name == player) {
                p.cards.clear();
                std::istringstream iss(cards);
                std::string card;
                while (iss >> card) p.cards.push_back(card);
                p.isThinking = false;
                p.lastAction = "Hit (Value: " + std::to_string(value) + ")";
                break;
            }
        }
        state.addLogEntry(player + " hits -> " + std::to_string(value));
        screen.Post(Event::Custom);
    }

    void onPlayerStand(const std::string& player, int value) override {
        std::lock_guard<std::mutex> lock(state.mutex);
        for (PlayerDisplayInfo& p : state.players) {
            if (p.name == player) { p.isThinking = false; p.lastAction = "Stand (" + std::to_string(value) + ")"; break; }
        }
        state.addLogEntry(player + " stands at " + std::to_string(value));
        screen.Post(Event::Custom);
    }

    void onPlayerBust(const std::string& player) override {
        std::lock_guard<std::mutex> lock(state.mutex);
        for (PlayerDisplayInfo& p : state.players) {
            if (p.name == player) { p.lastAction = "BUST!"; break; }
        }
        state.addLogEntry(player + " busts!");
        screen.Post(Event::Custom);
    }

    void onPlayerDoubleDown(const std::string& player, int newBet) override {
        std::lock_guard<std::mutex> lock(state.mutex);
        for (PlayerDisplayInfo& p : state.players) {
            if (p.name == player) { p.currentBet = newBet; p.isThinking = false; p.lastAction = "Double Down! (Bet: " + std::to_string(newBet) + ")"; break; }
        }
        state.addLogEntry(player + " doubles down (bet: " + std::to_string(newBet) + ")");
        screen.Post(Event::Custom);
    }

    void onDealerReveals(const std::string& cards, int value) override {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.showDealerHoleCard = true;
        state.dealer.cards.clear();
        std::istringstream iss(cards);
        std::string card;
        while (iss >> card) state.dealer.cards.push_back(card);
        state.dealer.lastAction = "Value: " + std::to_string(value);
        state.addLogEntry("Dealer reveals: " + std::to_string(value));
        screen.Post(Event::Custom);
    }

    void onDealerHits(const std::string& cards, int value) override {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.dealer.cards.clear();
        std::istringstream iss(cards);
        std::string card;
        while (iss >> card) state.dealer.cards.push_back(card);
        state.dealer.lastAction = "Hits -> " + std::to_string(value);
        state.addLogEntry("Dealer hits -> " + std::to_string(value));
        screen.Post(Event::Custom);
    }

    void onDealerBust() override {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.dealer.lastAction = "BUST!";
        state.addLogEntry("Dealer busts!");
        screen.Post(Event::Custom);
    }

    void onDealerStand(int value) override {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.dealer.lastAction = "Stands at " + std::to_string(value);
        state.addLogEntry("Dealer stands at " + std::to_string(value));
        screen.Post(Event::Custom);
    }

    void onBlackjackResult(const std::string& player, const std::string& outcome, int chips) override {
        std::lock_guard<std::mutex> lock(state.mutex);
        std::string sign = chips >= 0 ? "+" : "";
        state.addLogEntry(player + ": " + outcome + " (" + sign + std::to_string(chips) + ")");
        screen.Post(Event::Custom);
    }

    void onPlayerBustedOut(const std::string& player) override {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.addLogEntry(player + " is busted out!");
        screen.Post(Event::Custom);
    }
};

// ============================================================================
// Blackjack TUI Delegate
// ============================================================================

class BlackjackTuiDelegate : public IDecisionDelegate {
    BlackjackGameState& state;
    ScreenInteractive& screen;
public:
    BlackjackTuiDelegate(BlackjackGameState& s, ScreenInteractive& scr) : state(s), screen(scr) {}

    PokerAction getPokerAction(const BettingContext&) override { return PokerAction::fold(); }

    BlackjackAction getBlackjackAction(const BlackjackContext& context) override {
        {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.waitingForHumanBjAction = true;
            state.humanBjContext = context;
            state.humanBjResponse.reset();
        }
        screen.Post(Event::Custom);
        std::unique_lock<std::mutex> lock(state.mutex);
        state.humanBjInputCV.wait(lock, [&] { return state.humanBjResponse.has_value() || state.quitRequested; });
        state.waitingForHumanBjAction = false;
        if (state.quitRequested) return BlackjackAction::stand();
        return state.humanBjResponse.value();
    }

    int getBlackjackBet(const BlackjackBetContext& context) override {
        {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.waitingForHumanBet = true;
            state.humanBetContext = context;
            state.humanBetResponse.reset();
        }
        screen.Post(Event::Custom);
        std::unique_lock<std::mutex> lock(state.mutex);
        state.humanBetInputCV.wait(lock, [&] { return state.humanBetResponse.has_value() || state.quitRequested; });
        state.waitingForHumanBet = false;
        if (state.quitRequested) return context.minBet;
        return state.humanBetResponse.value();
    }
};

// ============================================================================
// Blackjack Screen
// ============================================================================

static Element renderPauseOverlay(int selection) {
    std::vector<std::string> options = {"Continue", "Exit to Menu", "Exit ChipStack"};
    Elements items;
    for (int i = 0; i < 3; i++) {
        Element item = text("  " + options[i] + "  ");
        if (i == selection)
            item = item | bold | color(Color::Green) | inverted;
        else
            item = item | color(Color::GrayLight);
        items.push_back(item);
    }
    return vbox({
        text("") | center,
        text("  PAUSED  ") | bold | center | color(Color::Yellow),
        text("") | center,
        vbox(items) | center,
        text("") | center,
        text("  Arrow keys + Enter  ") | dim | center,
    }) | border | center | clear_under;
}

bool runBlackjackScreen(const GameConfig& config) {
    ScreenInteractive screen = ScreenInteractive::Fullscreen();
    BlackjackGameState state;
    state.minBet = config.minBet;
    state.maxBet = config.maxBet;

    Player human(config.playerName, config.startingChips);
    std::vector<BotPlayer> bots;
    bots.reserve(config.bots.size());
    for (const BotConfig& bc : config.bots)
        bots.emplace_back(bc.name, config.startingChips, bc.riskTolerance, bc.aggressiveness, bc.difficulty);

    BlackjackTable table;
    table.addPlayer(&human);
    for (BotPlayer& bot : bots) table.addPlayer(&bot);

    BlackjackTuiObserver observer(state, screen);
    table.setObserver(&observer);
    BlackjackTuiDelegate bjDelegate(state, screen);
    human.setDelegate(&bjDelegate);

    ThinkingDelay thinkingDelay(true);
    table.preActionCallback = [&](IPlayer* player) {
        if (dynamic_cast<BotPlayer*>(player)) {
            std::chrono::milliseconds delay = thinkingDelay.generate();
            if (delay.count() > 0) {
                {
                    std::lock_guard<std::mutex> lock(state.mutex);
                    for (PlayerDisplayInfo& p : state.players)
                        if (p.name == player->getName()) { p.isThinking = true; break; }
                }
                screen.Post(Event::Custom);
                std::this_thread::sleep_for(delay);
                {
                    std::lock_guard<std::mutex> lock(state.mutex);
                    for (PlayerDisplayInfo& p : state.players)
                        if (p.name == player->getName()) { p.isThinking = false; break; }
                }
            }
        }
    };

    table.postPhaseCallback = [&](const std::string& announcement) {
        {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.dealerAnnouncement = announcement;
        }
        screen.Post(Event::Custom);
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.dealerAnnouncement.clear();
        }
        screen.Post(Event::Custom);
    };

    std::string betAmountStr = "";

    Component betInput = Input(&betAmountStr, "bet amount");

    Component component = CatchEvent(betInput, [&](Event event) {
        std::lock_guard<std::mutex> lock(state.mutex);

        if (state.paused) {
            if (event == Event::ArrowUp) {
                state.pauseSelection = (state.pauseSelection + 2) % 3;
                return true;
            }
            if (event == Event::ArrowDown) {
                state.pauseSelection = (state.pauseSelection + 1) % 3;
                return true;
            }
            if (event == Event::Return) {
                if (state.pauseSelection == 0) {
                    state.paused = false;
                } else if (state.pauseSelection == 1) {
                    state.paused = false;
                    state.quitRequested = true;
                    state.gameOver = true;
                    state.humanBjInputCV.notify_one();
                    state.humanBetInputCV.notify_one();
                    screen.ExitLoopClosure()();
                } else {
                    state.paused = false;
                    state.exitApp = true;
                    state.quitRequested = true;
                    state.gameOver = true;
                    state.humanBjInputCV.notify_one();
                    state.humanBetInputCV.notify_one();
                    screen.ExitLoopClosure()();
                }
                return true;
            }
            if (event == Event::Escape) {
                state.paused = false;
                return true;
            }
            return true;
        }

        if (event == Event::Escape) {
            state.paused = true;
            state.pauseSelection = 0;
            return true;
        }

        if (state.waitingForNextHand) {
            if (event == Event::Return) {
                state.humanBjResponse = BlackjackAction::stand();
                state.humanBjInputCV.notify_one();
                state.humanBetInputCV.notify_one();
                state.waitingForNextHand = false;
                return true;
            }
            if (event == Event::Character('q')) {
                state.paused = true;
                state.pauseSelection = 0;
                return true;
            }
        }

        if (state.waitingForHumanBet) {
            if (event == Event::Return && !betAmountStr.empty()) {
                try {
                    int bet = std::stoi(betAmountStr);
                    if (bet < state.humanBetContext.minBet) bet = state.humanBetContext.minBet;
                    if (bet > state.humanBetContext.maxBet) bet = state.humanBetContext.maxBet;
                    state.humanBetResponse = bet;
                    state.humanBetInputCV.notify_one();
                    betAmountStr = "";
                    return true;
                } catch (...) {}
            }
            return false;
        }

        if (state.waitingForHumanBjAction) {
            if (event == Event::Character('h') || event == Event::Character('H')) {
                state.humanBjResponse = BlackjackAction::hit();
                state.humanBjInputCV.notify_one();
                return true;
            }
            if (event == Event::Character('s') || event == Event::Character('S')) {
                state.humanBjResponse = BlackjackAction::stand();
                state.humanBjInputCV.notify_one();
                return true;
            }
            if (event == Event::Character('d') || event == Event::Character('D')) {
                if (state.humanBjContext.canDoubleDown) {
                    state.humanBjResponse = BlackjackAction::doubleDown();
                    state.humanBjInputCV.notify_one();
                    return true;
                }
            }
        }
        return false;
    });

    Component renderer = Renderer(component, [&] {
        std::lock_guard<std::mutex> lock(state.mutex);

        Element header = hbox({
            text(" Blackjack ") | bold | color(Color::Green),
            separator(),
            text(" Hand #" + std::to_string(state.handNumber) + " ") | bold,
            separator(),
            text(" Bets: " + std::to_string(state.minBet) + "-" + std::to_string(state.maxBet) + " "),
        }) | border;

        Element table = TableLayout::renderBlackjackTable(
            state.players, state.dealer, state.showDealerHoleCard);

        Element announcement;
        if (!state.dealerAnnouncement.empty()) {
            announcement = text(" Dealer: " + state.dealerAnnouncement + " ") | bold | center | color(Color::Yellow) | border;
        } else {
            announcement = text("");
        }

        Element actionBar;
        if (state.waitingForHumanBet) {
            actionBar = hbox({
                text(" Place your bet (" + std::to_string(state.humanBetContext.minBet) + "-" +
                     std::to_string(state.humanBetContext.maxBet) + "): ") | bold,
                component->Render() | size(WIDTH, EQUAL, 8) | border,
                text(" (Enter to confirm) ") | dim,
            }) | center;
        } else if (state.waitingForHumanBjAction) {
            Elements actions = {
                text(" [H]it ") | bold | color(Color::Green), text("  "),
                text(" [S]tand ") | bold | color(Color::Cyan), text("  "),
            };
            if (state.humanBjContext.canDoubleDown)
                actions.push_back(text(" [D]ouble Down ") | bold | color(Color::Magenta));
            actionBar = hbox(actions) | center;
        } else if (state.waitingForNextHand) {
            actionBar = text(" Press Enter for next hand, Esc to pause ") | bold | center | color(Color::Yellow);
        } else if (state.gameOver) {
            actionBar = text(" " + state.gameOverMessage + " ") | bold | center | color(Color::Red);
        } else {
            actionBar = text(" Waiting... ") | dim | center;
        }

        Elements logElems;
        for (const std::string& entry : state.actionLog)
            logElems.push_back(text("  " + entry) | dim);
        if (logElems.empty()) logElems.push_back(text("  Waiting for action...") | dim);

        Element content = vbox({
            header,
            table | flex,
            announcement,
            separator(),
            actionBar,
            separator(),
            vbox(logElems) | size(HEIGHT, EQUAL, 6),
        });

        if (state.paused) {
            content = dbox({
                content | dim,
                renderPauseOverlay(state.pauseSelection) | vcenter | hcenter,
            });
        }

        return content;
    });

    std::thread gameThread([&] {
        int handNum = 0;
        while (true) {
            {
                std::lock_guard<std::mutex> lock(state.mutex);
                if (state.quitRequested) break;
            }
            handNum++;
            if (config.handLimit > 0 && handNum > config.handLimit) {
                std::lock_guard<std::mutex> lock(state.mutex);
                state.gameOver = true;
                state.gameOverMessage = "Hand limit reached. Press Esc for menu.";
                screen.Post(Event::Custom);
                break;
            }

            {
                std::lock_guard<std::mutex> lock(state.mutex);
                state.handNumber = handNum;
                state.handComplete = false;
                state.showDealerHoleCard = false;
                state.dealer = PlayerDisplayInfo();
                state.dealer.name = "Dealer";
                state.players.clear();
                const std::vector<IPlayer*>& allPlayers = table.getPlayers();
                for (size_t i = 0; i < allPlayers.size(); i++) {
                    PlayerDisplayInfo info;
                    info.name = allPlayers[i]->getName();
                    info.chips = allPlayers[i]->getChips();
                    info.isHuman = (allPlayers[i] == &human);
                    info.showCards = true;
                    state.players.push_back(info);
                }
            }
            screen.Post(Event::Custom);

            table.playHand(config.minBet, config.maxBet);

            {
                std::lock_guard<std::mutex> lock(state.mutex);
                for (size_t i = 0; i < table.getPlayers().size(); i++) {
                    if (i < state.players.size())
                        state.players[i].chips = table.getPlayers()[i]->getChips();
                }
            }

            table.removeBustedPlayers();

            {
                std::lock_guard<std::mutex> lock(state.mutex);
                if (human.isBustedOut()) {
                    state.gameOver = true;
                    state.gameOverMessage = "You're busted out! Game over. Press Esc for menu.";
                    screen.Post(Event::Custom);
                    break;
                }
                if (table.getPlayerCount() < 1) {
                    state.gameOver = true;
                    state.gameOverMessage = "All opponents busted! You win! Press Esc for menu.";
                    screen.Post(Event::Custom);
                    break;
                }
            }

            {
                std::unique_lock<std::mutex> lock(state.mutex);
                state.waitingForNextHand = true;
                screen.Post(Event::Custom);
                state.humanBjInputCV.wait(lock, [&] { return state.humanBjResponse.has_value() || state.quitRequested; });
                state.waitingForNextHand = false;
                state.humanBjResponse.reset();
                if (state.quitRequested) break;
            }
        }
    });

    screen.Loop(renderer);
    {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.quitRequested = true;
        state.humanBjInputCV.notify_one();
        state.humanBetInputCV.notify_one();
    }
    gameThread.join();
    return state.exitApp;
}

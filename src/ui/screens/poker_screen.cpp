#include "poker_screen.hpp"
#include "ui/tui_app.hpp"
#include "ui/game_state.hpp"
#include "ui/card_renderer.hpp"
#include "ui/components/player_seat.hpp"
#include "ui/layout/table_layout.hpp"
#include "core/poker/poker_table.hpp"
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
// Poker TUI Observer
// ============================================================================

class PokerTuiObserver : public IGameObserver {
    PokerGameState& state;
    ScreenInteractive& screen;
public:
    PokerTuiObserver(PokerGameState& s, ScreenInteractive& scr) : state(s), screen(scr) {}

    void onHandStarted(const std::string& dealer, int sb, int bb) override {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.phaseName = "Dealing...";
        state.addLogEntry("Hand #" + std::to_string(state.handNumber) + " - Dealer: " + dealer);
        screen.Post(Event::Custom);
    }

    void onPlayerCards(const std::string& player, const std::string& c1, const std::string& c2) override {
        std::lock_guard<std::mutex> lock(state.mutex);
        for (PlayerDisplayInfo& p : state.players) {
            if (p.name == player) {
                p.cards = {c1, c2};
                break;
            }
        }
        screen.Post(Event::Custom);
    }

    void onBlindsPosted(const std::string& player, int amount, const std::string& blindType, int potTotal, int playerChips) override {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.pot = potTotal;
        for (PlayerDisplayInfo& p : state.players) {
            if (p.name == player) {
                p.currentBet = amount;
                p.chips = playerChips;
                p.lastAction = blindType + " (" + std::to_string(amount) + ")";
                break;
            }
        }
        state.addLogEntry(player + " posts " + blindType + " (" + std::to_string(amount) + ")");
        screen.Post(Event::Custom);
    }

    void onPhaseChanged(const std::string& phase, const std::string& cards) override {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.phaseName = phase;
        for (PlayerDisplayInfo& p : state.players) { p.currentBet = 0; p.lastAction = ""; }
        state.communityCards.clear();
        if (!cards.empty()) {
            std::string data = cards;
            const std::string prefix = "Community: ";
            if (data.substr(0, prefix.size()) == prefix)
                data = data.substr(prefix.size());
            std::istringstream iss(data);
            std::string token;
            while (iss >> token)
                state.communityCards.push_back(token);
        }
        state.addLogEntry("--- " + phase + " ---");
        screen.Post(Event::Custom);
    }

    void onPlayerAction(const std::string& player, const std::string& action, int amount, int potTotal, int playerChips) override {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.pot = potTotal;
        for (PlayerDisplayInfo& p : state.players) {
            if (p.name == player) {
                p.isThinking = false;
                p.chips = playerChips;
                if (action == "Fold") { p.isFolded = true; p.lastAction = "Fold"; }
                else if (action == "Check") { p.lastAction = "Check"; }
                else if (action == "Call") { p.lastAction = "Call " + std::to_string(amount); p.currentBet += amount; }
                else if (action == "Raise") { p.lastAction = "Raise to " + std::to_string(amount); p.currentBet = amount; }
                else if (action == "All-In") { p.isAllIn = true; p.lastAction = "ALL-IN " + std::to_string(amount); p.currentBet = amount; }
                break;
            }
        }
        std::string logMsg = player + " " + action;
        if (amount > 0) logMsg += " " + std::to_string(amount);
        state.addLogEntry(logMsg);
        screen.Post(Event::Custom);
    }

    void onPotWon(const std::string& winner, int amount, const std::string& rank) override {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.addLogEntry(winner + " wins " + std::to_string(amount) + " (" + rank + ")");
        state.pot = 0;
        state.handComplete = true;
        screen.Post(Event::Custom);
    }

    void onSplitPot(const std::vector<std::string>& winners, int share, const std::string& rank) override {
        std::lock_guard<std::mutex> lock(state.mutex);
        std::string names;
        for (size_t i = 0; i < winners.size(); i++) { if (i > 0) names += ", "; names += winners[i]; }
        state.addLogEntry("Split pot: " + names + " (" + std::to_string(share) + " each, " + rank + ")");
        state.pot = 0;
        state.handComplete = true;
        screen.Post(Event::Custom);
    }

    void onShowdownBegin(const std::string& cards) override {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.phaseName = "Showdown";
        state.addLogEntry("--- Showdown ---");
        for (PlayerDisplayInfo& p : state.players) { if (!p.isFolded) p.showCards = true; }
        screen.Post(Event::Custom);
    }

    void onShowdownReveal(const std::string& player, const std::string& cards, const std::string& rank) override {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.addLogEntry(player + ": " + rank);
        screen.Post(Event::Custom);
    }

    void onPlayerBustedOut(const std::string& player) override {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.addLogEntry(player + " is busted out!");
        screen.Post(Event::Custom);
    }
};

// ============================================================================
// Poker TUI Delegate
// ============================================================================

class PokerTuiDelegate : public IDecisionDelegate {
    PokerGameState& state;
    ScreenInteractive& screen;
public:
    PokerTuiDelegate(PokerGameState& s, ScreenInteractive& scr) : state(s), screen(scr) {}

    PokerAction getPokerAction(const BettingContext& context) override {
        {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.waitingForHumanAction = true;
            state.humanContext = context;
            state.humanResponse.reset();
        }
        screen.Post(Event::Custom);
        std::unique_lock<std::mutex> lock(state.mutex);
        state.humanInputCV.wait(lock, [&] { return state.humanResponse.has_value() || state.quitRequested; });
        state.waitingForHumanAction = false;
        if (state.quitRequested) return PokerAction::fold();
        return state.humanResponse.value();
    }

    BlackjackAction getBlackjackAction(const BlackjackContext&) override { return BlackjackAction::stand(); }
    int getBlackjackBet(const BlackjackBetContext& ctx) override { return ctx.minBet; }
};

// ============================================================================
// Poker Screen
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

bool runPokerScreen(const GameConfig& config) {
    ScreenInteractive screen = ScreenInteractive::Fullscreen();
    PokerGameState state;
    state.smallBlind = config.smallBlind;
    state.bigBlind = config.bigBlind;

    Player human(config.playerName, config.startingChips);
    std::vector<BotPlayer> bots;
    bots.reserve(config.bots.size());
    for (const BotConfig& bc : config.bots)
        bots.emplace_back(bc.name, config.startingChips, bc.riskTolerance, bc.aggressiveness, bc.difficulty);

    PokerTable table(config.smallBlind, config.bigBlind);
    table.addPlayer(&human);
    for (BotPlayer& bot : bots) table.addPlayer(&bot);

    PokerTuiObserver observer(state, screen);
    table.setObserver(&observer);
    PokerTuiDelegate delegate(state, screen); 
    human.setDelegate(&delegate);

    ThinkingDelay thinkingDelay(true);
    table.preActionCallback = [&](IPlayer* player) {
        if (dynamic_cast<BotPlayer*>(player)) {
            std::chrono::milliseconds delay = thinkingDelay.generate();
            if (delay.count() > 0) {
                {
                    std::lock_guard<std::mutex> lock(state.mutex);
                    for (PlayerDisplayInfo& p : state.players)
                        if (p.name == player->getName()) { p.isThinking = true; p.isCurrentTurn = true; break; }
                }
                screen.Post(Event::Custom);
                std::this_thread::sleep_for(delay);
                {
                    std::lock_guard<std::mutex> lock(state.mutex);
                    for (PlayerDisplayInfo& p : state.players)
                        if (p.name == player->getName()) { p.isThinking = false; break; }
                }
            }
        } else {
            std::lock_guard<std::mutex> lock(state.mutex);
            for (PlayerDisplayInfo& p : state.players)
                if (p.name == player->getName()) p.isCurrentTurn = true;
            screen.Post(Event::Custom);
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

    std::string raiseAmountStr = "";
    int raiseAmountVal = 0;

    Component raiseInput = Input(&raiseAmountStr, "amount");

    Component component = CatchEvent(raiseInput, [&](Event event) {
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
                    state.humanInputCV.notify_one();
                    screen.ExitLoopClosure()();
                } else {
                    state.paused = false;
                    state.exitApp = true;
                    state.quitRequested = true;
                    state.gameOver = true;
                    state.humanInputCV.notify_one();
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
                state.humanResponse = PokerAction::check();
                state.humanInputCV.notify_one();
                state.waitingForNextHand = false;
                return true;
            }
            if (event == Event::Character('q')) {
                state.paused = true;
                state.pauseSelection = 0;
                return true;
            }
        }

        if (!state.waitingForHumanAction) return false;

        if (event == Event::Character('f') || event == Event::Character('F')) {
            state.humanResponse = PokerAction::fold();
            state.humanInputCV.notify_one();
            return true;
        }
        if (event == Event::Character('c') || event == Event::Character('C')) {
            if (state.humanContext.costToCall == 0)
                state.humanResponse = PokerAction::check();
            else
                state.humanResponse = PokerAction::call();
            state.humanInputCV.notify_one();
            return true;
        }
        if (event == Event::Character('a') || event == Event::Character('A')) {
            state.humanResponse = PokerAction::allIn();
            state.humanInputCV.notify_one();
            return true;
        }
        if (event == Event::Return && !raiseAmountStr.empty()) {
            try {
                raiseAmountVal = std::stoi(raiseAmountStr);
                if (raiseAmountVal > 0) {
                    state.humanResponse = PokerAction::raise(raiseAmountVal);
                    state.humanInputCV.notify_one();
                    raiseAmountStr = "";
                    return true;
                }
            } catch (...) {}
        }
        return false;
    });

    Component renderer = Renderer(component, [&] {
        std::lock_guard<std::mutex> lock(state.mutex);

        Element header = hbox({
            text(" Poker ") | bold | color(Color::Green),
            separator(),
            text(" Hand #" + std::to_string(state.handNumber) + " ") | bold,
            separator(),
            text(" Blinds " + std::to_string(state.smallBlind) + "/" + std::to_string(state.bigBlind) + " "),
            separator(),
            text(" Pot: " + std::to_string(state.pot) + " ") | bold | color(Color::Yellow),
            separator(),
            text(" " + state.phaseName + " ") | color(Color::Cyan),
        }) | border;

        Element table = TableLayout::renderPokerTable(
            state.players, state.communityCards, state.pot, state.phaseName);

        Element announcement;
        if (!state.dealerAnnouncement.empty()) {
            announcement = text(" Dealer: " + state.dealerAnnouncement + " ") | bold | center | color(Color::Yellow) | border;
        } else {
            announcement = text("");
        }

        Element actionBar;
        if (state.waitingForHumanAction) {
            std::string callLabel = state.humanContext.costToCall == 0
                ? "[C]heck" : "[C]all " + std::to_string(state.humanContext.costToCall);
            actionBar = hbox({
                text(" [F]old ") | bold | color(Color::Red), text("  "),
                text(" " + callLabel + " ") | bold | color(Color::Green), text("  "),
                text(" [A]ll-In ") | bold | color(Color::Magenta), text("  "),
                text(" Raise: "),
                component->Render() | size(WIDTH, EQUAL, 8) | border,
                text(" (Enter to raise) ") | dim,
            }) | center;
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
                state.gameOverMessage = "Hand limit reached (" + std::to_string(config.handLimit) + " hands). Press Esc for menu.";
                screen.Post(Event::Custom);
                break;
            }

            {
                std::lock_guard<std::mutex> lock(state.mutex);
                state.handNumber = handNum;
                state.handComplete = false;
                state.communityCards.clear();
                state.pot = 0;
                state.players.clear();
                const std::vector<IPlayer*>& allPlayers = table.getPlayers();
                for (size_t i = 0; i < allPlayers.size(); i++) {
                    PlayerDisplayInfo info;
                    info.name = allPlayers[i]->getName();
                    info.chips = allPlayers[i]->getChips();
                    info.isHuman = (allPlayers[i] == &human);
                    info.showCards = info.isHuman;
                    info.isDealer = (static_cast<int>(i) == table.getDealerButton());
                    state.players.push_back(info);
                }
            }
            screen.Post(Event::Custom);

            table.playHand();

            {
                std::lock_guard<std::mutex> lock(state.mutex);
                for (PlayerDisplayInfo& p : state.players) {
                    p.isCurrentTurn = false;
                    p.isThinking = false;
                }
                for (size_t i = 0; i < table.getPlayers().size(); i++) {
                    if (i < state.players.size())
                        state.players[i].chips = table.getPlayers()[i]->getChips();
                }
                state.pot = table.getPot();
                state.handComplete = true;
            }
            screen.Post(Event::Custom);

            {
                std::unique_lock<std::mutex> lock(state.mutex);
                state.waitingForNextHand = true;
                screen.Post(Event::Custom);
                state.humanInputCV.wait(lock, [&] { return state.humanResponse.has_value() || state.quitRequested; });
                state.waitingForNextHand = false;
                state.humanResponse.reset();
                if (state.quitRequested) break;
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
                if (table.getPlayerCount() < 2) {
                    state.gameOver = true;
                    state.gameOverMessage = "You're the last one standing! You win! Press Esc for menu.";
                    screen.Post(Event::Custom);
                    break;
                }
            }
        }
    });

    screen.Loop(renderer);
    {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.quitRequested = true;
        state.humanInputCV.notify_one();
    }
    gameThread.join();
    return state.exitApp;
}

#include "tui_app.hpp"
#include "screens/poker_screen.hpp"
#include "screens/blackjack_screen.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

// ============================================================================
// Title banner (shared by menus)
// ============================================================================

static Element titleBanner() {
    return vbox({
        text("") | center,
        text("  ____ _     _       ____  _             _    ") | bold | color(Color::Green) | center,
        text(" / ___| |__ (_)_ __ / ___|| |_ __ _  ___| | __") | bold | color(Color::Green) | center,
        text("| |   | '_ \\| | '_ \\\\___ \\| __/ _` |/ __| |/ /") | bold | color(Color::Green) | center,
        text("| |___| | | | | |_) |___) | || (_| | (__|   < ") | bold | color(Color::Green) | center,
        text(" \\____|_| |_|_| .__/|____/ \\__\\__,_|\\___|_|\\_\\") | bold | color(Color::Green) | center,
        text("              |_|                              ") | bold | color(Color::Green) | center,
        text("") | center,
    });
}

// ============================================================================
// Bot presets
// ============================================================================

static const float BOT_PRESETS[][2] = {
    {0.25f, 0.15f}, // Novice
    {0.60f, 0.50f}, // Good
    {0.50f, 0.65f}, // Great
    {0.75f, 0.55f}, // Professional
};

std::vector<BotConfig> TuiApp::makePresetBots(
    const std::vector<std::pair<std::string, Difficulty>>& specs) {
    std::vector<BotConfig> bots;
    for (auto& [name, diff] : specs) {
        int idx = static_cast<int>(diff);
        bots.push_back({name, diff, BOT_PRESETS[idx][0], BOT_PRESETS[idx][1]});
    }
    return bots;
}

// ============================================================================
// Main loop
// ============================================================================

void TuiApp::run() {
    while (true) {
        if (!showMainMenu()) return;

        bool scenarioOk;
        if (config.gameType == GameConfig::Poker) {
            scenarioOk = showPokerScenarios();
        } else {
            scenarioOk = showBlackjackScenarios();
        }
        if (!scenarioOk) continue;

        if (!showChipSelect()) continue;

        if (runGame()) return;
    }
}

// ============================================================================
// Menus
// ============================================================================

bool TuiApp::showMainMenu() {
    ScreenInteractive screen = ScreenInteractive::Fullscreen();
    int selected = 0;
    bool quit = false;

    std::vector<std::string> entries = {
        "  Poker (NL Hold'em)  ",
        "  Blackjack           ",
        "  Quit                ",
    };

    Component menu = Menu(&entries, &selected);

    Component component = CatchEvent(menu, [&](Event event) {
        if (event == Event::Return) {
            if (selected == 0) {
                config.gameType = GameConfig::Poker;
                screen.ExitLoopClosure()();
            } else if (selected == 1) {
                config.gameType = GameConfig::Blackjack;
                screen.ExitLoopClosure()();
            } else {
                quit = true;
                screen.ExitLoopClosure()();
            }
            return true;
        }
        if (event == Event::Character('q')) {
            quit = true;
            screen.ExitLoopClosure()();
            return true;
        }
        return false;
    });

    Component renderer = Renderer(component, [&] {
        return vbox({
            titleBanner(),
            separator(),
            text("  Select a game:") | bold,
            text(""),
            component->Render() | border | size(WIDTH, EQUAL, 30) | center,
            text(""),
            text("  Use arrow keys + Enter to select, 'q' to quit") | dim | center,
        }) | center;
    });

    screen.Loop(renderer);
    return !quit;
}

bool TuiApp::showPokerScenarios() {
    ScreenInteractive screen = ScreenInteractive::Fullscreen();
    int selected = 0;
    bool goBack = false;
    bool customSelected = false;

    std::vector<std::string> entries = {
        "  Casual Home Game    5/10 blinds, 3 Novice bots       ",
        "  Friday Night        10/20 blinds, Good+Great+Pro     ",
        "  Shark Tank          25/50 blinds, Great+2 Pro        ",
        "  The Grind           10/20 blinds, 4 mixed bots       ",
        "  Custom Game         Configure everything             ",
        "  Back                                                 ",
    };

    Component menu = Menu(&entries, &selected);

    Component component = CatchEvent(menu, [&](Event event) {
        if (event == Event::Return) {
            switch (selected) {
                case 0:
                    config.smallBlind = 5; config.bigBlind = 10; config.handLimit = 0;
                    config.bots = makePresetBots({{"Timmy", Difficulty::Novice}, {"Bobby", Difficulty::Novice}, {"Sally", Difficulty::Novice}});
                    break;
                case 1:
                    config.smallBlind = 10; config.bigBlind = 20; config.handLimit = 0;
                    config.bots = makePresetBots({{"Rocky", Difficulty::Good}, {"Sharp", Difficulty::Great}, {"Ace", Difficulty::Professional}});
                    break;
                case 2:
                    config.smallBlind = 25; config.bigBlind = 50; config.handLimit = 0;
                    config.bots = makePresetBots({{"Sharp", Difficulty::Great}, {"Ace", Difficulty::Professional}, {"Viper", Difficulty::Professional}});
                    break;
                case 3:
                    config.smallBlind = 10; config.bigBlind = 20; config.handLimit = 0;
                    config.bots = makePresetBots({{"Timmy", Difficulty::Novice}, {"Rocky", Difficulty::Good}, {"Sharp", Difficulty::Great}, {"Ace", Difficulty::Professional}});
                    break;
                case 4:
                    customSelected = true; screen.ExitLoopClosure()(); return true;
                case 5:
                    goBack = true; screen.ExitLoopClosure()(); return true;
            }
            screen.ExitLoopClosure()(); return true;
        }
        if (event == Event::Escape) { goBack = true; screen.ExitLoopClosure()(); return true; }
        return false;
    });

    Component renderer = Renderer(component, [&] {
        return vbox({
            titleBanner(), separator(),
            text("  Poker - Choose a scenario:") | bold, text(""),
            component->Render() | border | center, text(""),
            text("  Enter to select, Esc to go back") | dim | center,
        }) | center;
    });

    screen.Loop(renderer);
    if (goBack) return false;
    if (customSelected) return showCustomPokerConfig();
    return true;
}

bool TuiApp::showBlackjackScenarios() {
    ScreenInteractive screen = ScreenInteractive::Fullscreen();
    int selected = 0;
    bool goBack = false;
    bool customSelected = false;

    std::vector<std::string> entries = {
        "  Low Stakes          10-100 bets, 2 Novice bots       ",
        "  Standard Table      25-500 bets, Good+Great+Pro      ",
        "  High Roller         100-2000 bets, 2 Pro bots        ",
        "  Custom Game         Configure everything             ",
        "  Back                                                 ",
    };

    Component menu = Menu(&entries, &selected);

    Component component = CatchEvent(menu, [&](Event event) {
        if (event == Event::Return) {
            switch (selected) {
                case 0:
                    config.minBet = 10; config.maxBet = 100; config.handLimit = 0;
                    config.bots = makePresetBots({{"Timmy", Difficulty::Novice}, {"Bobby", Difficulty::Novice}});
                    break;
                case 1:
                    config.minBet = 25; config.maxBet = 500; config.handLimit = 0;
                    config.bots = makePresetBots({{"Rocky", Difficulty::Good}, {"Sharp", Difficulty::Great}, {"Ace", Difficulty::Professional}});
                    break;
                case 2:
                    config.minBet = 100; config.maxBet = 2000; config.handLimit = 0;
                    config.bots = makePresetBots({{"Ace", Difficulty::Professional}, {"Viper", Difficulty::Professional}});
                    break;
                case 3:
                    customSelected = true; screen.ExitLoopClosure()(); return true;
                case 4:
                    goBack = true; screen.ExitLoopClosure()(); return true;
            }
            screen.ExitLoopClosure()(); return true;
        }
        if (event == Event::Escape) { goBack = true; screen.ExitLoopClosure()(); return true; }
        return false;
    });

    Component renderer = Renderer(component, [&] {
        return vbox({
            titleBanner(), separator(),
            text("  Blackjack - Choose a scenario:") | bold, text(""),
            component->Render() | border | center, text(""),
            text("  Enter to select, Esc to go back") | dim | center,
        }) | center;
    });

    screen.Loop(renderer);
    if (goBack) return false;
    if (customSelected) return showCustomBlackjackConfig();
    return true;
}

static const char* difficultyName(int d) {
    switch (d) {
        case 0: return "Novice"; case 1: return "Good";
        case 2: return "Great"; case 3: return "Pro";
    }
    return "?";
}

static const std::vector<std::string> BOT_NAMES = {
    "Timmy", "Rocky", "Sharp", "Ace", "Viper"
};

bool TuiApp::showCustomPokerConfig() {
    ScreenInteractive screen = ScreenInteractive::Fullscreen();
    bool goBack = false;
    int numBots = 3;
    int botDiffs[5] = {0, 1, 2, 3, 3};
    int sb = 10, bb = 20, handLimitVal = 0;
    bool unlimited = true;

    Component numBotsSlider = Slider("  Bots: ", &numBots, 1, 5, 1);
    Component sbSlider = Slider("  Small Blind: ", &sb, 5, 100, 5);
    Component bbSlider = Slider("  Big Blind: ", &bb, 10, 200, 10);
    Component handLimitSlider = Slider("  Hand Limit: ", &handLimitVal, 1, 50, 1);
    Component unlimitedCheck = Checkbox("  Unlimited hands", &unlimited);

    std::vector<Component> diffSliders;
    for (int i = 0; i < 5; i++)
        diffSliders.push_back(Slider("    " + BOT_NAMES[i] + ": ", &botDiffs[i], 0, 3, 1));

    Component startBtn = Button("  Start Game  ", [&] {
        config.smallBlind = sb; config.bigBlind = bb;
        config.handLimit = unlimited ? 0 : handLimitVal;
        config.bots.clear();
        for (int i = 0; i < numBots; i++) {
            Difficulty d = static_cast<Difficulty>(botDiffs[i]);
            int idx = botDiffs[i];
            config.bots.push_back({BOT_NAMES[i], d, BOT_PRESETS[idx][0], BOT_PRESETS[idx][1]});
        }
        screen.ExitLoopClosure()();
    });
    Component backBtn = Button("  Back  ", [&] { goBack = true; screen.ExitLoopClosure()(); });

    Component layout = Container::Vertical({numBotsSlider, Container::Vertical(diffSliders),
        sbSlider, bbSlider, unlimitedCheck, handLimitSlider, Container::Horizontal({startBtn, backBtn})});

    Component component = CatchEvent(layout, [&](Event event) {
        if (event == Event::Escape) { goBack = true; screen.ExitLoopClosure()(); return true; }
        return false;
    });

    Component renderer = Renderer(component, [&] {
        Elements botElements;
        for (int i = 0; i < numBots; i++)
            botElements.push_back(hbox({diffSliders[i]->Render() | size(WIDTH, EQUAL, 40),
                text("  (" + std::string(difficultyName(botDiffs[i])) + ")")}));
        Elements content = {titleBanner(), separator(), text("  Custom Poker Game") | bold, text(""),
            numBotsSlider->Render() | size(WIDTH, EQUAL, 40), text(""), text("  Bot Difficulties:") | bold};
        for (Element& e : botElements) content.push_back(e);
        content.push_back(text(""));
        content.push_back(sbSlider->Render() | size(WIDTH, EQUAL, 40));
        content.push_back(bbSlider->Render() | size(WIDTH, EQUAL, 40));
        content.push_back(text(""));
        content.push_back(unlimitedCheck->Render());
        if (!unlimited) content.push_back(handLimitSlider->Render() | size(WIDTH, EQUAL, 40));
        content.push_back(text(""));
        content.push_back(hbox({startBtn->Render(), text("  "), backBtn->Render()}) | center);
        content.push_back(text("")); content.push_back(text("  Esc to go back") | dim | center);
        return vbox(content) | center;
    });

    screen.Loop(renderer);
    return !goBack;
}

bool TuiApp::showCustomBlackjackConfig() {
    ScreenInteractive screen = ScreenInteractive::Fullscreen();
    bool goBack = false;
    int numBots = 2;
    int botDiffs[5] = {0, 1, 2, 3, 3};
    int minB = 10, maxB = 500, handLimitVal = 0;
    bool unlimited = true;

    Component numBotsSlider = Slider("  Bots: ", &numBots, 1, 5, 1);
    Component minSlider = Slider("  Min Bet: ", &minB, 5, 500, 5);
    Component maxSlider = Slider("  Max Bet: ", &maxB, 100, 5000, 100);
    Component handLimitSlider = Slider("  Hand Limit: ", &handLimitVal, 1, 50, 1);
    Component unlimitedCheck = Checkbox("  Unlimited hands", &unlimited);

    std::vector<Component> diffSliders;
    for (int i = 0; i < 5; i++)
        diffSliders.push_back(Slider("    " + BOT_NAMES[i] + ": ", &botDiffs[i], 0, 3, 1));

    Component startBtn = Button("  Start Game  ", [&] {
        config.minBet = minB; config.maxBet = maxB;
        config.handLimit = unlimited ? 0 : handLimitVal;
        config.bots.clear();
        for (int i = 0; i < numBots; i++) {
            Difficulty d = static_cast<Difficulty>(botDiffs[i]);
            int idx = botDiffs[i];
            config.bots.push_back({BOT_NAMES[i], d, BOT_PRESETS[idx][0], BOT_PRESETS[idx][1]});
        }
        screen.ExitLoopClosure()();
    });
    Component backBtn = Button("  Back  ", [&] { goBack = true; screen.ExitLoopClosure()(); });

    Component layout = Container::Vertical({numBotsSlider, Container::Vertical(diffSliders),
        minSlider, maxSlider, unlimitedCheck, handLimitSlider, Container::Horizontal({startBtn, backBtn})});

    Component component = CatchEvent(layout, [&](Event event) {
        if (event == Event::Escape) { goBack = true; screen.ExitLoopClosure()(); return true; }
        return false;
    });

    Component renderer = Renderer(component, [&] {
        Elements botElements;
        for (int i = 0; i < numBots; i++)
            botElements.push_back(hbox({diffSliders[i]->Render() | size(WIDTH, EQUAL, 40),
                text("  (" + std::string(difficultyName(botDiffs[i])) + ")")}));
        Elements content = {titleBanner(), separator(), text("  Custom Blackjack Game") | bold, text(""),
            numBotsSlider->Render() | size(WIDTH, EQUAL, 40), text(""), text("  Bot Difficulties:") | bold};
        for (Element& e : botElements) content.push_back(e);
        content.push_back(text(""));
        content.push_back(minSlider->Render() | size(WIDTH, EQUAL, 40));
        content.push_back(maxSlider->Render() | size(WIDTH, EQUAL, 40));
        content.push_back(text(""));
        content.push_back(unlimitedCheck->Render());
        if (!unlimited) content.push_back(handLimitSlider->Render() | size(WIDTH, EQUAL, 40));
        content.push_back(text(""));
        content.push_back(hbox({startBtn->Render(), text("  "), backBtn->Render()}) | center);
        content.push_back(text("")); content.push_back(text("  Esc to go back") | dim | center);
        return vbox(content) | center;
    });

    screen.Loop(renderer);
    return !goBack;
}

bool TuiApp::showChipSelect() {
    ScreenInteractive screen = ScreenInteractive::Fullscreen();
    bool goBack = false;
    int chipIndex = 18;
    std::string playerName = "You";

    Component nameInput = Input(&playerName, "Enter your name");
    Component chipSlider = Slider("", &chipIndex, 0, 198, 1);

    Component startBtn = Button("  Start Game  ", [&] {
        config.playerName = playerName;
        config.startingChips = chipIndex * 50 + 100;
        screen.ExitLoopClosure()();
    });
    Component backBtn = Button("  Back  ", [&] { goBack = true; screen.ExitLoopClosure()(); });

    Component layout = Container::Vertical({nameInput, chipSlider, Container::Horizontal({startBtn, backBtn})});
    Component component = CatchEvent(layout, [&](Event event) {
        if (event == Event::Escape) { goBack = true; screen.ExitLoopClosure()(); return true; }
        return false;
    });

    Component renderer = Renderer(component, [&] {
        int chips = chipIndex * 50 + 100;
        std::string gameLabel = config.gameType == GameConfig::Poker ? "Poker" : "Blackjack";
        std::string botList;
        for (size_t i = 0; i < config.bots.size(); i++) {
            if (i > 0) botList += ", ";
            botList += config.bots[i].name;
        }
        return vbox({
            titleBanner(), separator(),
            text("  Game: " + gameLabel) | bold,
            text("  Opponents: " + botList) | dim, text(""), separator(), text(""),
            text("  Player Name:") | bold,
            nameInput->Render() | size(WIDTH, EQUAL, 30) | center, text(""),
            text("  Starting Chips: " + std::to_string(chips)) | bold | center,
            chipSlider->Render() | size(WIDTH, EQUAL, 50) | center, text(""),
            hbox({text("  100") | dim, filler(), text("10000  ") | dim}) | size(WIDTH, EQUAL, 50) | center,
            text(""),
            hbox({startBtn->Render(), text("  "), backBtn->Render()}) | center,
            text(""), text("  Esc to go back") | dim | center,
        }) | center;
    });

    screen.Loop(renderer);
    return !goBack;
}

bool TuiApp::runGame() {
    if (config.gameType == GameConfig::Poker) {
        return runPokerGame();
    } else {
        return runBlackjackGame();
    }
}

bool TuiApp::runPokerGame() {
    return runPokerScreen(config);
}

bool TuiApp::runBlackjackGame() {
    return runBlackjackScreen(config);
}

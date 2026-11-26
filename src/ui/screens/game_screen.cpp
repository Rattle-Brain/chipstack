#include "game_screen.hpp"
#include <thread>

using namespace ftxui;

GameScreen::GameScreen(const GameConfig& config)
		: config(config), screen(ScreenInteractive::Fullscreen()),
			amountInput(Input(&amountText, "amount")) {
	for (const BotConfig& bot : config.bots) {
		bots.push_back(
				std::make_unique<BotPlayer>(
						bot.name, config.startingChips, bot.personality
				)
		);
	}
}

bool GameScreen::run() {
	Component events = CatchEvent(amountInput, [this](Event event) {
		return handleEvent(event);
	});
	Component root = Renderer(events, [this] { return render(); });
	std::thread gameThread([this] { playHands(); });
	screen.Loop(root);
	stopGame();
	gameThread.join();
	return session().exitApp;
}

void GameScreen::playHands() {
	for (int hand = 1; !session().isQuitting(); hand++) {
		if (handLimitReached(hand)) {
			std::string limit = std::to_string(config.handLimit);
			endGame("Hand limit reached (" + limit + " hands). Press Esc for menu.");
			return;
		}
		playHand(hand);
		if (!session().waitForNextHand()) return;
		table().removeBustedPlayers();
		if (std::optional<std::string> result = finalResult()) {
			endGame(*result);
			return;
		}
	}
}

bool GameScreen::handLimitReached(int handNumber) const {
	return config.handLimit > 0 && handNumber > config.handLimit;
}

void GameScreen::playHand(int handNumber) {
	session().update([&] { prepareHandView(handNumber); });
	table().playHand();
	session().update([&] { finishHandView(); });
}

void GameScreen::prepareHandView(int handNumber) {
	GameSession& state = session();
	state.handNumber = handNumber;
	state.seats.clear();
	for (const Player* player : table().getSeatedPlayers()) {
		SeatView seat;
		seat.name = player->getName();
		seat.chips = player->getChips();
		seat.isHuman = player == humanPlayer;
		seat.showCards = seat.isHuman;
		decorateSeat(seat, state.seats.size());
		state.seats.push_back(seat);
	}
	resetTableView();
}

void GameScreen::finishHandView() {
	std::vector<const Player*> players = table().getSeatedPlayers();
	std::vector<SeatView>& seats = session().seats;
	for (size_t i = 0; i < seats.size() && i < players.size(); i++) {
		seats[i].chips = players[i]->getChips();
		seats[i].isCurrentTurn = false;
		seats[i].isThinking = false;
	}
}

std::optional<std::string> GameScreen::finalResult() {
	if (humanPlayer->isBustedOut()) {
		return "You're busted out! Game over. Press Esc for menu.";
	}
	if (table().getPlayerCount() < 2) {
		return "You're the last one standing! You win! Press Esc for menu.";
	}
	return std::nullopt;
}

void GameScreen::endGame(const std::string& message) {
	session().update([&] {
		session().gameOver = true;
		session().gameOverMessage = message;
	});
}

void GameScreen::stopGame() {
	std::lock_guard<std::mutex> lock(session().mutex);
	session().requestQuit(false);
}

bool GameScreen::handleEvent(const Event& event) {
	std::lock_guard<std::mutex> lock(session().mutex);
	GameSession& state = session();
	if (state.pauseMenu.isOpen()) return handlePauseEvent(event);
	bool betweenHands = state.waitingForNextHand;
	if (event == Event::Escape || (betweenHands && isKey(event, 'q'))) {
		state.pauseMenu.show();
		return true;
	}
	if (betweenHands && event == Event::Return) {
		state.continueToNextHand();
		return true;
	}
	return handleDecisionEvent(event);
}

bool GameScreen::handlePauseEvent(const Event& event) {
	std::optional<PauseChoice> choice = session().pauseMenu.handleEvent(event);
	if (choice && *choice != PauseChoice::Continue) {
		session().requestQuit(*choice == PauseChoice::ExitApp);
		screen.ExitLoopClosure()();
	}
	return true;
}

bool GameScreen::isKey(const Event& event, char key) {
	char lower = std::tolower(key);
	char upper = std::toupper(key);
	return event == Event::Character(lower) || event == Event::Character(upper);
}

std::optional<int> GameScreen::takeEnteredAmount() {
	try {
		int amount = std::stoi(amountText);
		amountText.clear();
		return amount;
	} catch (const std::exception&) {
		return std::nullopt;
	}
}

Element GameScreen::renderAmountInput() {
	return amountInput->Render() | size(WIDTH, EQUAL, 8) | border;
}

Element GameScreen::render() {
	std::lock_guard<std::mutex> lock(session().mutex);
	Element content = vbox({
			renderHeader(),
			renderTable() | flex,
			renderAnnouncement(),
			separator(),
			renderActionBar(),
			separator(),
			renderLog(),
	});
	if (!session().pauseMenu.isOpen()) return content;
	return dbox({content | dim, session().pauseMenu.render()});
}

Element GameScreen::renderHeader() {
	std::string hand = " Hand #" + std::to_string(session().handNumber) + " ";
	Elements items = {
			text(" " + gameTitle() + " ") | bold | color(Color::Green),
			separator(),
			text(hand) | bold,
	};
	for (Element& detail : headerDetails()) {
		items.push_back(separator());
		items.push_back(detail);
	}
	return hbox(items) | border;
}

Element GameScreen::renderAnnouncement() {
	const std::string& announcement = session().dealerAnnouncement;
	if (announcement.empty()) return text("");
	return text(" Dealer: " + announcement + " ") | bold | center
			| color(Color::Yellow) | border;
}

Element GameScreen::renderActionBar() {
	GameSession& state = session();
	if (Element decisionBar = renderDecisionBar()) return decisionBar;
	if (state.waitingForNextHand) {
		return text(" Press Enter for next hand, Esc to pause ") | bold | center
				| color(Color::Yellow);
	}
	if (state.gameOver) {
		return text(" " + state.gameOverMessage + " ") | bold | center
				| color(Color::Red);
	}
	return text(" Waiting... ") | dim | center;
}

Element GameScreen::renderLog() {
	Elements lines;
	for (const std::string& entry : session().actionLog) {
		lines.push_back(text("  " + entry) | dim);
	}
	if (lines.empty()) lines.push_back(text("  Waiting for action...") | dim);
	return vbox(lines) | size(HEIGHT, EQUAL, GameSession::VISIBLE_LOG_LINES);
}

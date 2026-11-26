#include "game_session.hpp"
#include <ftxui/component/event.hpp>

bool GameSession::waitForNextHand() {
	std::unique_lock<std::mutex> lock(mutex);
	waitingForNextHand = true;
	continueRequested = false;
	redraw();
	inputArrived.wait(lock, [this] {
		return continueRequested || quitRequested;
	});
	waitingForNextHand = false;
	return !quitRequested;
}

void GameSession::continueToNextHand() {
	continueRequested = true;
	inputArrived.notify_all();
}

void GameSession::requestQuit(bool exitApplication) {
	exitApp = exitApp || exitApplication;
	quitRequested = true;
	gameOver = true;
	inputArrived.notify_all();
}

bool GameSession::isQuitting() {
	std::lock_guard<std::mutex> lock(mutex);
	return quitRequested;
}

void GameSession::pauseFor(std::chrono::milliseconds duration) {
	std::unique_lock<std::mutex> lock(mutex);
	inputArrived.wait_for(lock, duration, [this] { return quitRequested; });
}

void GameSession::redraw() {
	screen.Post(ftxui::Event::Custom);
}

SeatView& GameSession::seatOf(const std::string& name) {
	for (SeatView& seat : seats) {
		if (seat.name == name) return seat;
	}
	return unknownSeat;
}

void GameSession::markTurn(const std::string& name) {
	for (SeatView& seat : seats) {
		seat.isCurrentTurn = seat.name == name;
	}
}

void GameSession::addLogEntry(const std::string& entry) {
	actionLog.push_back(entry);
	if (actionLog.size() > VISIBLE_LOG_LINES) actionLog.pop_front();
}

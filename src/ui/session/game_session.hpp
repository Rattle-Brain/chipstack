#pragma once

#include "seat_view.hpp"
#include "ui/widgets/pause_menu.hpp"
#include <chrono>
#include <condition_variable>
#include <deque>
#include <ftxui/component/screen_interactive.hpp>
#include <mutex>
#include <optional>
#include <vector>

template <class Context, class Response>
struct PendingDecision {
	bool waiting = false;
	Context context{};
	std::optional<Response> response;
};

class GameSession {
public:
	static constexpr size_t VISIBLE_LOG_LINES = 6;

	explicit GameSession(ftxui::ScreenInteractive& screen) : screen(screen) {}

	template <class Mutation>
	void update(Mutation&& mutate) {
		{
			std::lock_guard<std::mutex> lock(mutex);
			mutate();
		}
		redraw();
	}

	template <class Context, class Response>
	Response awaitDecision(
			PendingDecision<Context, Response>& decision, const Context& context,
			Response fallback
	) {
		std::unique_lock<std::mutex> lock(mutex);
		decision = {true, context, std::nullopt};
		redraw();
		inputArrived.wait(lock, [&] { return decision.response || quitRequested; });
		decision.waiting = false;
		return quitRequested ? fallback : *decision.response;
	}

	template <class Context, class Response>
	void respond(PendingDecision<Context, Response>& decision, Response answer) {
		decision.response = answer;
		inputArrived.notify_all();
	}

	bool waitForNextHand();
	void continueToNextHand();
	void requestQuit(bool exitApplication);
	bool isQuitting();
	void pauseFor(std::chrono::milliseconds duration);
	void redraw();

	SeatView& seatOf(const std::string& name);
	void markTurn(const std::string& name);
	void addLogEntry(const std::string& entry);

	std::mutex mutex;
	std::vector<SeatView> seats;
	std::deque<std::string> actionLog;
	std::string dealerAnnouncement;
	std::string gameOverMessage;
	PauseMenu pauseMenu;
	int handNumber = 0;
	bool gameOver = false;
	bool waitingForNextHand = false;
	bool exitApp = false;

private:
	ftxui::ScreenInteractive& screen;
	std::condition_variable inputArrived;
	bool continueRequested = false;
	bool quitRequested = false;
	SeatView unknownSeat;
};

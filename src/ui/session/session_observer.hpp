#pragma once

#include "ai/thinking_delay.hpp"
#include "core/table/table_observer.hpp"

template <class ObserverBase, class Session>
class SessionObserver : public ObserverBase {
public:
	explicit SessionObserver(Session& session) : session(session) {}

	void onBeforeDecision(const Player& player) override {
		bool isHuman = false;
		session.update([&] {
			session.markTurn(player.getName());
			isHuman = session.seatOf(player.getName()).isHuman;
		});
		if (!isHuman) simulateThinking(player.getName());
	}

	void onAnnouncement(const std::string& message) override {
		session.update([&] { session.dealerAnnouncement = message; });
		session.pauseFor(std::chrono::milliseconds(1500));
		session.update([&] { session.dealerAnnouncement.clear(); });
	}

	void onPlayerBustedOut(const Player& player) override {
		log(player.getName() + " is busted out!");
	}

protected:
	void log(const std::string& entry) {
		session.update([&] { session.addLogEntry(entry); });
	}

	Session& session;

private:
	void simulateThinking(const std::string& name) {
		std::chrono::milliseconds delay = thinkingDelay.generate();
		if (delay.count() == 0) return;
		session.update([&] { session.seatOf(name).isThinking = true; });
		session.pauseFor(delay);
		session.update([&] { session.seatOf(name).isThinking = false; });
	}

	ThinkingDelay thinkingDelay;
};

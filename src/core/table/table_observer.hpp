#pragma once

#include "core/players/player.hpp"
#include <string>

class TableObserver {
public:
	virtual ~TableObserver() = default;

	virtual void onBeforeDecision(const Player& player) {}
	virtual void onAnnouncement(const std::string& message) {}
	virtual void onPlayerBustedOut(const Player& player) {}
	virtual void onError(const std::string& message) {}
};

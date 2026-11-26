#pragma once

#include "ai/bot_personality.hpp"
#include <string>
#include <vector>

struct BotConfig {
	std::string name;
	BotPersonality personality;
};

struct StakeLimits {
	int low;
	int high;
};

struct GameConfig {
	std::string playerName = "You";
	int startingChips = 1000;
	std::vector<BotConfig> bots;
	StakeLimits stakes{10, 20};
	int handLimit = 0;
};

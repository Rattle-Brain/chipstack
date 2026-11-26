#pragma once

#include "game_config.hpp"
#include <string>
#include <utility>
#include <vector>

struct Scenario {
	std::string title;
	std::string detail;
	StakeLimits stakes;
	std::vector<std::pair<std::string, Difficulty>> bots;
};

struct StakeSlider {
	std::string label;
	int min;
	int max;
	int step;
};

struct GameDefinition {
	std::string name;
	std::string menuTitle;
	std::vector<Scenario> scenarios;
	StakeSlider lowStake;
	StakeSlider highStake;
	StakeLimits defaultStakes;
	int defaultBotCount;
	bool (*launch)(const GameConfig& config);
};

const std::vector<GameDefinition>& gameCatalog();
void applyScenario(const Scenario& scenario, GameConfig& config);

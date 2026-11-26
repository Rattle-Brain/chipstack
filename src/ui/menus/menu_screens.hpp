#pragma once

#include "ui/game_config.hpp"
#include <optional>
#include <string>
#include <vector>

std::string menuEntry(const std::string& title, const std::string& detail = "");

std::optional<size_t> chooseFromMenu(
		const std::string& heading, const std::vector<std::string>& entries,
		const std::string& hint
);

bool configurePlayer(GameConfig& config, const std::string& gameName);

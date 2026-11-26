#include "tui_app.hpp"
#include "game_catalog.hpp"
#include "menus/custom_game_form.hpp"
#include "menus/menu_screens.hpp"

static const GameDefinition* chooseGame() {
	const std::vector<GameDefinition>& games = gameCatalog();
	std::vector<std::string> entries;
	for (const GameDefinition& game : games) {
		entries.push_back(menuEntry(game.menuTitle));
	}
	entries.push_back(menuEntry("Quit"));
	std::optional<size_t> choice = chooseFromMenu(
			"Select a game:", entries, "Arrow keys + Enter to select, Esc to quit"
	);
	if (!choice || *choice >= games.size()) return nullptr;
	return &games[*choice];
}

static bool chooseScenario(const GameDefinition& game, GameConfig& config) {
	std::vector<std::string> entries;
	for (const Scenario& scenario : game.scenarios) {
		entries.push_back(menuEntry(scenario.title, scenario.detail));
	}
	entries.push_back(menuEntry("Custom Game", "Configure everything"));
	entries.push_back(menuEntry("Back"));
	std::optional<size_t> choice = chooseFromMenu(
			game.name + " - Choose a scenario:", entries,
			"Enter to select, Esc to go back"
	);
	size_t scenarioCount = game.scenarios.size();
	if (!choice || *choice > scenarioCount) return false;
	if (*choice == scenarioCount) return CustomGameForm(game).run(config);
	applyScenario(game.scenarios[*choice], config);
	return true;
}

void runTuiApp() {
	GameConfig config;
	while (const GameDefinition* game = chooseGame()) {
		if (!chooseScenario(*game, config)) continue;
		if (!configurePlayer(config, game->name)) continue;
		if (game->launch(config)) return;
	}
}

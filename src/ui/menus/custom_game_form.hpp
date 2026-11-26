#pragma once

#include "form_screen.hpp"
#include "ui/game_catalog.hpp"
#include <array>

class CustomGameForm {
public:
	explicit CustomGameForm(const GameDefinition& game);

	bool run(GameConfig& config);

private:
	static constexpr int MAX_BOTS = 5;

	ftxui::Component buildLayout();
	ftxui::Element render();
	ftxui::Elements renderBotRows();
	void applyTo(GameConfig& config) const;

	const GameDefinition& game;
	FormScreen form;
	int botCount;
	std::array<int, MAX_BOTS> difficulties = {0, 1, 2, 3, 3};
	StakeLimits stakes;
	int handLimit = 10;
	bool unlimitedHands = true;
	ftxui::Component botCountSlider;
	std::vector<ftxui::Component> difficultySliders;
	ftxui::Component lowStakeSlider;
	ftxui::Component highStakeSlider;
	ftxui::Component unlimitedCheckbox;
	ftxui::Component handLimitSlider;
	ftxui::Component startButton;
	ftxui::Component backButton;
};

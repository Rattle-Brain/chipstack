#include "custom_game_form.hpp"

using namespace ftxui;

static const char* BOT_NAMES[] = {"Timmy", "Rocky", "Sharp", "Ace", "Viper"};
static constexpr int SLIDER_WIDTH = 40;

static Component stakeSlider(const StakeSlider& spec, int* value) {
	return Slider("  " + spec.label + ": ", value, spec.min, spec.max, spec.step);
}

CustomGameForm::CustomGameForm(const GameDefinition& game)
		: game(game), botCount(game.defaultBotCount), stakes(game.defaultStakes) {}

bool CustomGameForm::run(GameConfig& config) {
	Component layout = buildLayout();
	if (!form.run(layout, [this] { return render(); })) return false;
	applyTo(config);
	return true;
}

Component CustomGameForm::buildLayout() {
	botCountSlider = Slider("  Bots: ", &botCount, 1, MAX_BOTS, 1);
	for (int i = 0; i < MAX_BOTS; i++) {
		std::string label = "    " + std::string(BOT_NAMES[i]) + ": ";
		difficultySliders.push_back(Slider(label, &difficulties[i], 0, 3, 1));
	}
	lowStakeSlider = stakeSlider(game.lowStake, &stakes.low);
	highStakeSlider = stakeSlider(game.highStake, &stakes.high);
	unlimitedCheckbox = Checkbox("  Unlimited hands", &unlimitedHands);
	handLimitSlider = Slider("  Hand Limit: ", &handLimit, 1, 50, 1);
	startButton = form.startButton();
	backButton = form.backButton();
	return Container::Vertical({
			botCountSlider,
			Container::Vertical(difficultySliders),
			lowStakeSlider,
			highStakeSlider,
			unlimitedCheckbox,
			handLimitSlider,
			Container::Horizontal({startButton, backButton}),
	});
}

Element CustomGameForm::render() {
	Decorator sliderWidth = size(WIDTH, EQUAL, SLIDER_WIDTH);
	Elements body = {
			botCountSlider->Render() | sliderWidth,
			text(""),
			text("  Bot Difficulties:") | bold,
	};
	Elements botRows = renderBotRows();
	body.insert(body.end(), botRows.begin(), botRows.end());
	body.push_back(text(""));
	body.push_back(lowStakeSlider->Render() | sliderWidth);
	body.push_back(highStakeSlider->Render() | sliderWidth);
	body.push_back(text(""));
	body.push_back(unlimitedCheckbox->Render());
	if (!unlimitedHands) body.push_back(handLimitSlider->Render() | sliderWidth);
	body.push_back(text(""));
	body.push_back(form.renderButtons(startButton, backButton));
	return renderMenuPage(
			"Custom " + game.name + " Game", body, "Esc to go back"
	);
}

Elements CustomGameForm::renderBotRows() {
	Elements rows;
	for (int i = 0; i < botCount; i++) {
		Difficulty difficulty = static_cast<Difficulty>(difficulties[i]);
		rows.push_back(hbox({
				difficultySliders[i]->Render() | size(WIDTH, EQUAL, SLIDER_WIDTH),
				text("  (" + std::string(difficultyName(difficulty)) + ")"),
		}));
	}
	return rows;
}

void CustomGameForm::applyTo(GameConfig& config) const {
	config.stakes = stakes;
	config.handLimit = unlimitedHands ? 0 : handLimit;
	config.bots.clear();
	for (int i = 0; i < botCount; i++) {
		Difficulty difficulty = static_cast<Difficulty>(difficulties[i]);
		config.bots.push_back({BOT_NAMES[i], presetPersonality(difficulty)});
	}
}

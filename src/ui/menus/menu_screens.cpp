#include "menu_screens.hpp"
#include "core/common/text.hpp"
#include "form_screen.hpp"

using namespace ftxui;

std::string menuEntry(const std::string& title, const std::string& detail) {
	constexpr size_t TITLE_WIDTH = 20;
	std::string padding(TITLE_WIDTH - std::min(TITLE_WIDTH, title.size()), ' ');
	return "  " + title + padding + detail + "  ";
}

std::optional<size_t> chooseFromMenu(
		const std::string& heading, const std::vector<std::string>& entries,
		const std::string& hint
) {
	FormScreen form;
	int selected = 0;
	MenuOption option;
	option.on_enter = form.closer();
	Component menu = Menu(&entries, &selected, option);
	bool chosen = form.run(menu, [&] {
		return renderMenuPage(heading, {menu->Render() | border | center}, hint);
	});
	if (!chosen) return std::nullopt;
	return selected;
}

static std::string botNames(const GameConfig& config) {
	std::vector<std::string> names;
	for (const BotConfig& bot : config.bots) {
		names.push_back(bot.name);
	}
	return join(names, ", ");
}

bool configurePlayer(GameConfig& config, const std::string& gameName) {
	FormScreen form;
	std::string name = config.playerName;
	int chips = config.startingChips;
	Component nameInput = Input(&name, "Enter your name");
	Component chipSlider = Slider("", &chips, 100, 10000, 50);
	Component start = form.startButton();
	Component back = form.backButton();
	Component layout = Container::Vertical(
			{nameInput, chipSlider, Container::Horizontal({start, back})}
	);
	bool confirmed = form.run(layout, [&] {
		Element chipScale =
				hbox({text("  100") | dim, filler(), text("10000  ") | dim});
		return renderMenuPage(
				"Game: " + gameName,
				{
						text("  Opponents: " + botNames(config)) | dim,
						text(""),
						separator(),
						text(""),
						text("  Player Name:") | bold,
						nameInput->Render() | size(WIDTH, EQUAL, 30) | center,
						text(""),
						text("  Starting Chips: " + std::to_string(chips)) | bold | center,
						chipSlider->Render() | size(WIDTH, EQUAL, 50) | center,
						chipScale | size(WIDTH, EQUAL, 50) | center,
						text(""),
						form.renderButtons(start, back),
				},
				"Esc to go back"
		);
	});
	if (!confirmed) return false;
	config.playerName = name;
	config.startingChips = chips;
	return true;
}

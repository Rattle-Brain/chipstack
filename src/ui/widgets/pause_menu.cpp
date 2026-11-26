#include "pause_menu.hpp"

using namespace ftxui;

static const char* OPTION_LABELS[] = {
		"Continue", "Exit to Menu", "Exit ChipStack"
};
static constexpr int OPTION_COUNT = 3;

void PauseMenu::show() {
	open = true;
	selection = 0;
}

std::optional<PauseChoice> PauseMenu::handleEvent(const Event& event) {
	if (event == Event::ArrowUp) moveSelection(-1);
	if (event == Event::ArrowDown) moveSelection(1);
	if (event == Event::Escape) open = false;
	if (event != Event::Return) return std::nullopt;
	open = false;
	return static_cast<PauseChoice>(selection);
}

void PauseMenu::moveSelection(int step) {
	selection = (selection + step + OPTION_COUNT) % OPTION_COUNT;
}

Element PauseMenu::render() const {
	Elements options;
	for (int i = 0; i < OPTION_COUNT; i++) {
		Element option = text("  " + std::string(OPTION_LABELS[i]) + "  ");
		if (i == selection) {
			options.push_back(option | bold | color(Color::Green) | inverted);
		} else {
			options.push_back(option | color(Color::GrayLight));
		}
	}
	Element dialog = vbox({
			text(""),
			text("  PAUSED  ") | bold | center | color(Color::Yellow),
			text(""),
			vbox(options) | center,
			text(""),
			text("  Arrow keys + Enter  ") | dim | center,
	});
	return dialog | border | clear_under | center;
}

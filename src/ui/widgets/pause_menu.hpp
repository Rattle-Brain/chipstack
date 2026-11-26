#pragma once

#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <optional>

enum class PauseChoice { Continue, ExitToMenu, ExitApp };

class PauseMenu {
public:
	bool isOpen() const { return open; }

	void show();
	std::optional<PauseChoice> handleEvent(const ftxui::Event& event);
	ftxui::Element render() const;

private:
	void moveSelection(int step);

	bool open = false;
	int selection = 0;
};

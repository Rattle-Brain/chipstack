#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <functional>
#include <string>

ftxui::Element renderMenuPage(
		const std::string& heading, ftxui::Elements body, const std::string& hint
);

class FormScreen {
public:
	FormScreen();

	bool run(ftxui::Component form, std::function<ftxui::Element()> render);
	std::function<void()> closer();
	ftxui::Component startButton();
	ftxui::Component backButton();
	ftxui::Element renderButtons(
			const ftxui::Component& start, const ftxui::Component& back
	) const;

private:
	void cancel();

	ftxui::ScreenInteractive screen;
	bool cancelled = false;
};

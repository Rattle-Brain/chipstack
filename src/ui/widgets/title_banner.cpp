#include "title_banner.hpp"

using namespace ftxui;

static const char* BANNER_LINES[] = {
		R"(  ____ _     _       ____  _             _    )",
		R"( / ___| |__ (_)_ __ / ___|| |_ __ _  ___| | __)",
		R"(| |   | '_ \| | '_ \\___ \| __/ _` |/ __| |/ /)",
		R"(| |___| | | | | |_) |___) | || (_| | (__|   < )",
		R"( \____|_| |_|_| .__/|____/ \__\__,_|\___|_|\_\)",
		R"(              |_|                              )",
};

Element renderTitleBanner() {
	Elements lines = {text("")};
	for (const char* line : BANNER_LINES) {
		lines.push_back(text(line) | bold | color(Color::Green) | center);
	}
	lines.push_back(text(""));
	return vbox(lines);
}

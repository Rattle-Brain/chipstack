#include "form_screen.hpp"
#include "ui/widgets/title_banner.hpp"

using namespace ftxui;

Element renderMenuPage(
		const std::string& heading, Elements body, const std::string& hint
) {
	Elements page = {
			renderTitleBanner(),
			separator(),
			text("  " + heading) | bold,
			text(""),
	};
	page.insert(page.end(), body.begin(), body.end());
	page.push_back(text(""));
	page.push_back(text("  " + hint) | dim | center);
	return vbox(page) | center;
}

FormScreen::FormScreen() : screen(ScreenInteractive::Fullscreen()) {}

bool FormScreen::run(Component form, std::function<Element()> render) {
	Component cancellable = CatchEvent(form, [this](Event event) {
		if (event != Event::Escape) return false;
		cancel();
		return true;
	});
	screen.Loop(Renderer(cancellable, std::move(render)));
	return !cancelled;
}

std::function<void()> FormScreen::closer() {
	return screen.ExitLoopClosure();
}

Component FormScreen::startButton() {
	return Button("  Start Game  ", closer());
}

Component FormScreen::backButton() {
	return Button("  Back  ", [this] { cancel(); });
}

Element FormScreen::renderButtons(
		const Component& start, const Component& back
) const {
	return hbox({start->Render(), text("  "), back->Render()}) | center;
}

void FormScreen::cancel() {
	cancelled = true;
	closer()();
}

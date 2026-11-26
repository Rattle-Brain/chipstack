#include "card_view.hpp"

using namespace ftxui;

static Element cardFrame(Element face) {
	return face | border | size(WIDTH, EQUAL, 10) | size(HEIGHT, EQUAL, 7);
}

Element CardView::renderCard(const Card& card) {
	Decorator ink = color(card.isRed() ? Color::Red : Color::White);
	Element face = vbox({
			text(" " + card.rankLabel()) | ink,
			text(""),
			text(card.suitSymbol()) | bold | hcenter | ink,
			text(""),
			text(card.rankLabel() + " ") | align_right | ink,
	});
	return cardFrame(face) | bgcolor(Color::GrayDark);
}

Element CardView::renderFaceDown() {
	auto pattern = [] {
		return text("? ?") | hcenter;
	};
	Element face = vbox({text(""), pattern(), text(""), pattern(), text("")});
	return cardFrame(face) | bgcolor(Color::Blue) | color(Color::White);
}

Element CardView::renderEmptySlot() {
	return cardFrame(filler()) | dim;
}

Element CardView::renderHand(const Cards& cards, bool faceDown) {
	Elements views;
	for (const Card& card : cards) {
		views.push_back(faceDown ? renderFaceDown() : renderCard(card));
	}
	return hbox(views);
}

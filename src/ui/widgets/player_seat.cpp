#include "player_seat.hpp"
#include "card_view.hpp"

using namespace ftxui;

static std::string seatStatus(const SeatView& seat) {
	if (seat.isThinking) return "Thinking...";
	if (!seat.lastAction.empty()) return seat.lastAction;
	return seat.isFolded ? "Folded" : "";
}

static Color statusColor(const SeatView& seat) {
	if (seat.isFolded) return Color::GrayDark;
	if (seat.isAllIn) return Color::Red;
	return seat.isThinking ? Color::Yellow : Color::White;
}

Element renderPlayerSeat(const SeatView& seat) {
	Color nameColor = seat.isHuman ? Color::Yellow : Color::Cyan;
	std::string title = seat.name + (seat.isDealer ? " (D)" : "");
	Elements content = {
			text(title) | bold | color(nameColor),
			text(std::to_string(seat.chips) + " chips") | dim,
	};
	bool faceDown = !seat.showCards && !seat.isHuman;
	if (!seat.cards.empty()) {
		content.push_back(CardView::renderHand(seat.cards, faceDown));
	}
	if (seat.bet > 0) {
		std::string bet = "Bet: " + std::to_string(seat.bet);
		content.push_back(text(bet) | color(Color::Green));
	}
	std::string status = seatStatus(seat);
	if (!status.empty()) {
		content.push_back(text(status) | color(statusColor(seat)));
	}
	Element view = vbox(content) | border;
	if (seat.isCurrentTurn) view |= bgcolor(Color::RGB(30, 60, 30));
	if (seat.isFolded) view |= dim;
	return view | size(WIDTH, GREATER_THAN, 20);
}

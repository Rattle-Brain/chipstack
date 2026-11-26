#include "table_layout.hpp"
#include "ui/widgets/card_view.hpp"
#include "ui/widgets/player_seat.hpp"
#include <ftxui/dom/flexbox_config.hpp>

using namespace ftxui;

namespace {

using SeatRefs = std::vector<const SeatView*>;
using Justify = FlexboxConfig::JustifyContent;

struct SeatGroups {
	const SeatView* human = nullptr;
	SeatRefs bots;
};

SeatGroups groupSeats(const std::vector<SeatView>& seats) {
	SeatGroups groups;
	for (const SeatView& seat : seats) {
		if (seat.isHuman) {
			groups.human = &seat;
		} else {
			groups.bots.push_back(&seat);
		}
	}
	return groups;
}

Element renderSeatRow(const SeatRefs& seats, Justify justify) {
	Elements views;
	for (const SeatView* seat : seats) {
		views.push_back(renderPlayerSeat(*seat));
	}
	return flexbox(views, FlexboxConfig().Set(justify));
}

Element renderSideSeat(const SeatView* seat) {
	if (!seat) return filler() | size(WIDTH, EQUAL, 20);
	return renderPlayerSeat(*seat) | vcenter;
}

Element renderFelt(Elements rows) {
	return vbox(rows) | size(HEIGHT, GREATER_THAN, 18)
			| borderStyled(ROUNDED, Color::Green) | bgcolor(Color::RGB(0, 40, 0));
}

Element renderBoard(
		const Cards& communityCards, int pot, const std::string& street
) {
	Elements cards;
	for (const Card& card : communityCards) {
		cards.push_back(CardView::renderCard(card));
	}
	while (cards.size() < 5) {
		cards.push_back(CardView::renderEmptySlot());
	}
	Element area = vbox({
			text(street) | bold | center | color(Color::Cyan),
			hbox(cards) | center,
			text("Pot: " + std::to_string(pot)) | bold | center
					| color(Color::Yellow),
	});
	return area | center | flex;
}

Element renderDealerArea(const SeatView& dealer, bool holeCardVisible) {
	Elements cards;
	for (size_t i = 0; i < dealer.cards.size(); i++) {
		bool hidden = i == 1 && !holeCardVisible;
		Element view = CardView::renderCard(dealer.cards[i]);
		cards.push_back(hidden ? CardView::renderFaceDown() : view);
	}
	Element area = vbox({
			text("Dealer") | bold | center | color(Color::Cyan),
			hbox(cards) | center,
			text(dealer.lastAction) | dim | center,
	});
	return area | center;
}

} // namespace

Element TableLayout::renderPokerTable(
		const std::vector<SeatView>& seats, const Cards& communityCards, int pot,
		const std::string& streetName
) {
	SeatGroups groups = groupSeats(seats);
	SeatRefs top = groups.bots;
	const SeatView* left = nullptr;
	const SeatView* right = nullptr;
	if (top.size() >= 3) {
		left = top.front();
		right = top.back();
		top = SeatRefs(top.begin() + 1, top.end() - 1);
	}
	Element board = renderBoard(communityCards, pot, streetName);
	Element middle =
			left ? hbox({renderSideSeat(left), board, renderSideSeat(right)}) : board;
	Element bottom =
			groups.human ? renderPlayerSeat(*groups.human) | hcenter : text("");
	return renderFelt({
			renderSeatRow(top, Justify::SpaceEvenly),
			filler(),
			middle,
			filler(),
			bottom,
	});
}

Element TableLayout::renderBlackjackTable(
		const std::vector<SeatView>& seats, const SeatView& dealer,
		bool holeCardVisible
) {
	SeatGroups groups = groupSeats(seats);
	SeatRefs outer = groups.bots;
	SeatRefs inner;
	if (outer.size() >= 3) {
		inner = SeatRefs(outer.begin() + 1, outer.end() - 1);
		outer = {outer.front(), outer.back()};
	}
	if (groups.human) {
		inner.insert(inner.begin() + (inner.size() + 1) / 2, groups.human);
	}
	Justify outerJustify =
			outer.size() == 1 ? Justify::Center : Justify::SpaceBetween;
	return renderFelt({
			renderDealerArea(dealer, holeCardVisible),
			filler(),
			renderSeatRow(outer, outerJustify),
			filler(),
			renderSeatRow(inner, Justify::SpaceEvenly),
	});
}

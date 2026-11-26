#pragma once

#include "ui/session/seat_view.hpp"
#include <ftxui/dom/elements.hpp>
#include <vector>

namespace TableLayout {

ftxui::Element renderPokerTable(
		const std::vector<SeatView>& seats, const Cards& communityCards, int pot,
		const std::string& streetName
);

ftxui::Element renderBlackjackTable(
		const std::vector<SeatView>& seats, const SeatView& dealer,
		bool holeCardVisible
);

} // namespace TableLayout

#ifndef TABLE_LAYOUT_HPP
#define TABLE_LAYOUT_HPP

#include <ftxui/dom/elements.hpp>
#include <vector>
#include <string>

struct PlayerDisplayInfo;

namespace TableLayout {

ftxui::Element renderPokerTable(
    const std::vector<PlayerDisplayInfo>& players,
    const std::vector<std::string>& communityCards,
    int pot,
    const std::string& phaseName);

ftxui::Element renderBlackjackTable(
    const std::vector<PlayerDisplayInfo>& players,
    const PlayerDisplayInfo& dealer,
    bool showDealerHoleCard);

}

#endif

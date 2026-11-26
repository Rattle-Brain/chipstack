#ifndef CARD_RENDERER_HPP
#define CARD_RENDERER_HPP

#include <ftxui/dom/elements.hpp>
#include <string>
#include <vector>

namespace CardRenderer {

ftxui::Element renderCard(const std::string& cardStr, bool faceDown = false);
ftxui::Element renderCardRow(const std::vector<std::string>& cards, bool faceDown = false);
ftxui::Element renderEmptySlot();

bool isRedSuit(const std::string& cardStr);

}

#endif

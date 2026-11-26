#pragma once

#include "core/cards/card.hpp"
#include <ftxui/dom/elements.hpp>

namespace CardView {

ftxui::Element renderCard(const Card& card);
ftxui::Element renderFaceDown();
ftxui::Element renderEmptySlot();
ftxui::Element renderHand(const Cards& cards, bool faceDown = false);

} // namespace CardView

#include "player_seat.hpp"
#include "ui/game_state.hpp"
#include "ui/card_renderer.hpp"

using namespace ftxui;

Element renderPlayerSeat(const PlayerDisplayInfo& p) {
    Color nameColor = p.isHuman ? Color::Yellow : Color::Cyan;
    Color statusColor = Color::White;
    if (p.isFolded) statusColor = Color::GrayDark;
    else if (p.isAllIn) statusColor = Color::Red;
    else if (p.isThinking) statusColor = Color::Yellow;

    Elements cardElems;
    if (!p.cards.empty()) {
        for (const std::string& card : p.cards) {
            if (p.showCards || p.isHuman)
                cardElems.push_back(CardRenderer::renderCard(card));
            else
                cardElems.push_back(CardRenderer::renderCard(card, true));
        }
    }

    std::string statusStr;
    if (p.isThinking) statusStr = "Thinking...";
    else if (!p.lastAction.empty()) statusStr = p.lastAction;
    else if (p.isFolded) statusStr = "Folded";

    Elements content = {
        text(p.name + (p.isDealer ? " (D)" : "")) | bold | color(nameColor),
        text(std::to_string(p.chips) + " chips") | dim,
    };
    if (!cardElems.empty()) content.push_back(hbox(cardElems));
    if (p.currentBet > 0) content.push_back(text("Bet: " + std::to_string(p.currentBet)) | color(Color::Green));
    if (!statusStr.empty()) content.push_back(text(statusStr) | color(statusColor));

    Element elem = vbox(content) | border;
    if (p.isCurrentTurn) elem = elem | bgcolor(Color::RGB(30, 60, 30));
    if (p.isFolded) elem = elem | dim;
    return elem | size(WIDTH, EQUAL, 20);
}

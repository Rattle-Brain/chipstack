#include "table_layout.hpp"
#include "ui/game_state.hpp"
#include "ui/card_renderer.hpp"
#include "ui/components/player_seat.hpp"

#include <ftxui/dom/flexbox_config.hpp>

using namespace ftxui;

namespace TableLayout {

struct SeatPositions {
    std::vector<const PlayerDisplayInfo*> topRow;
    const PlayerDisplayInfo* leftSeat = nullptr;
    const PlayerDisplayInfo* rightSeat = nullptr;
    const PlayerDisplayInfo* humanSeat = nullptr;
};

static SeatPositions distributeSeats(const std::vector<PlayerDisplayInfo>& players) {
    SeatPositions pos;

    std::vector<const PlayerDisplayInfo*> bots;
    for (const PlayerDisplayInfo& p : players) {
        if (p.isHuman)
            pos.humanSeat = &p;
        else
            bots.push_back(&p);
    }

    int n = static_cast<int>(bots.size());
    switch (n) {
        case 1:
            pos.topRow = {bots[0]};
            break;
        case 2:
            pos.topRow = {bots[0], bots[1]};
            break;
        case 3:
            pos.leftSeat = bots[0];
            pos.topRow = {bots[1]};
            pos.rightSeat = bots[2];
            break;
        case 4:
            pos.leftSeat = bots[0];
            pos.topRow = {bots[1], bots[2]};
            pos.rightSeat = bots[3];
            break;
        case 5:
            pos.leftSeat = bots[0];
            pos.topRow = {bots[1], bots[2], bots[3]};
            pos.rightSeat = bots[4];
            break;
    }
    return pos;
}

Element renderPokerTable(
    const std::vector<PlayerDisplayInfo>& players,
    const std::vector<std::string>& communityCards,
    int pot,
    const std::string& phaseName)
{
    SeatPositions positions = distributeSeats(players);

    // Top row: bots at the far side of the table
    Elements topSeats;
    for (const PlayerDisplayInfo* p : positions.topRow)
        topSeats.push_back(renderPlayerSeat(*p));

    Element topRow = flexbox(topSeats,
        FlexboxConfig()
            .Set(FlexboxConfig::JustifyContent::SpaceEvenly)
            .Set(FlexboxConfig::AlignItems::FlexEnd));

    // Community cards + pot (center of the table)
    Elements ccElems;
    for (const std::string& card : communityCards)
        ccElems.push_back(CardRenderer::renderCard(card));
    while (static_cast<int>(ccElems.size()) < 5)
        ccElems.push_back(CardRenderer::renderEmptySlot());

    Element centerContent = vbox({
        text(phaseName) | bold | center | color(Color::Cyan),
        hbox(ccElems) | center,
        text("Pot: " + std::to_string(pot)) | bold | center | color(Color::Yellow),
    }) | center | flex;

    // Middle row: left seat, center, right seat
    Element middleRow;
    if (positions.leftSeat || positions.rightSeat) {
        Elements middleElems;
        if (positions.leftSeat)
            middleElems.push_back(renderPlayerSeat(*positions.leftSeat) | vcenter);
        else
            middleElems.push_back(filler() | size(WIDTH, EQUAL, 20));

        middleElems.push_back(centerContent);

        if (positions.rightSeat)
            middleElems.push_back(renderPlayerSeat(*positions.rightSeat) | vcenter);
        else
            middleElems.push_back(filler() | size(WIDTH, EQUAL, 20));

        middleRow = hbox(middleElems);
    } else {
        middleRow = centerContent;
    }

    // Bottom row: human player
    Element bottomRow = text("") | center;
    if (positions.humanSeat)
        bottomRow = renderPlayerSeat(*positions.humanSeat) | hcenter;

    // Compose the table
    Element tableContent = vbox({
        topRow,
        filler(),
        middleRow,
        filler(),
        bottomRow,
    }) | size(HEIGHT, GREATER_THAN, 18);

    return tableContent
        | borderStyled(ROUNDED, Color::Green)
        | bgcolor(Color::RGB(0, 40, 0));
}

static Element renderBlackjackSeat(const PlayerDisplayInfo& p) {
    Color nameColor = p.isHuman ? Color::Yellow : Color::Cyan;
    Color statusColor = p.isThinking ? Color::Yellow : Color::White;

    Elements cardElems;
    for (const std::string& card : p.cards)
        cardElems.push_back(CardRenderer::renderCard(card));

    std::string statusStr;
    if (p.isThinking) statusStr = "Thinking...";
    else if (!p.lastAction.empty()) statusStr = p.lastAction;

    Elements content = {
        text(p.name) | bold | color(nameColor),
        text(std::to_string(p.chips) + " chips") | dim,
    };
    if (!cardElems.empty()) content.push_back(hbox(cardElems));
    if (p.currentBet > 0)
        content.push_back(text("Bet: " + std::to_string(p.currentBet)) | color(Color::Green));
    if (!statusStr.empty())
        content.push_back(text(statusStr) | color(statusColor));

    Element elem = vbox(content) | border;
    if (p.isHuman) elem = elem | bgcolor(Color::RGB(30, 60, 30));
    return elem | size(WIDTH, GREATER_THAN, 20);
}

Element renderBlackjackTable(
    const std::vector<PlayerDisplayInfo>& players,
    const PlayerDisplayInfo& dealer,
    bool showDealerHoleCard)
{
    const PlayerDisplayInfo* humanSeat = nullptr;
    std::vector<const PlayerDisplayInfo*> bots;
    for (const PlayerDisplayInfo& p : players) {
        if (p.isHuman)
            humanSeat = &p;
        else
            bots.push_back(&p);
    }

    int n = static_cast<int>(bots.size());

    // Dealer area at the top
    Elements dealerCards;
    for (size_t i = 0; i < dealer.cards.size(); i++) {
        if (i == 1 && !showDealerHoleCard)
            dealerCards.push_back(CardRenderer::renderCard("", true));
        else
            dealerCards.push_back(CardRenderer::renderCard(dealer.cards[i]));
    }

    Element dealerArea = vbox({
        text("Dealer") | bold | center | color(Color::Cyan),
        hbox(dealerCards) | center,
        text(dealer.lastAction) | dim | center,
    }) | center;

    // Semicircle: split bots into outer tier (wide) and inner tier (narrow)
    std::vector<const PlayerDisplayInfo*> outerTier;
    std::vector<const PlayerDisplayInfo*> innerTier;

    switch (n) {
        case 1:
            outerTier = {bots[0]};
            innerTier = {humanSeat};
            break;
        case 2:
            outerTier = {bots[0], bots[1]};
            innerTier = {humanSeat};
            break;
        case 3:
            outerTier = {bots[0], bots[2]};
            innerTier = {bots[1], humanSeat};
            break;
        case 4:
            outerTier = {bots[0], bots[3]};
            innerTier = {bots[1], humanSeat, bots[2]};
            break;
        case 5:
            outerTier = {bots[0], bots[4]};
            innerTier = {bots[1], bots[2], humanSeat, bots[3]};
            break;
    }

    // Outer tier: seats pushed to far edges
    Element outerRow = text("");
    if (!outerTier.empty()) {
        Elements seats;
        for (const PlayerDisplayInfo* p : outerTier)
            seats.push_back(renderBlackjackSeat(*p));
        FlexboxConfig::JustifyContent justify = outerTier.size() == 1
            ? FlexboxConfig::JustifyContent::Center
            : FlexboxConfig::JustifyContent::SpaceBetween;
        outerRow = flexbox(seats,
            FlexboxConfig().Set(justify));
    }

    // Inner tier: seats closer to center
    Element innerRow = text("");
    if (!innerTier.empty()) {
        Elements seats;
        for (const PlayerDisplayInfo* p : innerTier)
            seats.push_back(renderBlackjackSeat(*p));
        innerRow = flexbox(seats,
            FlexboxConfig().Set(FlexboxConfig::JustifyContent::SpaceEvenly));
    }

    Element tableContent = vbox({
        dealerArea,
        filler(),
        outerRow,
        filler(),
        innerRow,
    }) | size(HEIGHT, GREATER_THAN, 18);

    return tableContent
        | borderStyled(ROUNDED, Color::Green)
        | bgcolor(Color::RGB(0, 40, 0));
}

}

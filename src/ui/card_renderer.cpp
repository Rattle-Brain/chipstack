#include "card_renderer.hpp"

using namespace ftxui;

namespace CardRenderer {

bool isRedSuit(const std::string& cardStr) {
    return cardStr.find("♥") != std::string::npos ||
           cardStr.find("♦") != std::string::npos;
}

static const int CARD_WIDTH = 10;
static const int CARD_HEIGHT = 7;
static const int INNER_WIDTH = CARD_WIDTH - 2;

static std::pair<std::string, std::string> parseCard(const std::string& cardStr) {
    for (const std::string& suit : {"♠", "♥", "♦", "♣"}) {
        size_t pos = cardStr.find(suit);
        if (pos != std::string::npos) {
            return {cardStr.substr(0, pos), suit};
        }
    }
    return {cardStr, ""};
}

static std::string padRight(const std::string& s, int width) {
    int len = static_cast<int>(s.size());
    if (len >= width) return s;
    return s + std::string(width - len, ' ');
}

static std::string padLeft(const std::string& s, int width) {
    int len = static_cast<int>(s.size());
    if (len >= width) return s;
    return std::string(width - len, ' ') + s;
}

static std::string centerStr(const std::string& s, int width) {
    int len = static_cast<int>(s.size());
    if (len >= width) return s;
    int left = (width - len) / 2;
    int right = width - len - left;
    return std::string(left, ' ') + s + std::string(right, ' ');
}

Element renderCard(const std::string& cardStr, bool faceDown) {
    if (faceDown) {
        std::string fill(INNER_WIDTH, ' ');
        std::string pattern = centerStr("? ?", INNER_WIDTH);
        Element content = vbox({
            text(fill),
            text(pattern),
            text(fill),
            text(pattern),
            text(fill),
        });
        return content
            | border
            | size(WIDTH, EQUAL, CARD_WIDTH)
            | size(HEIGHT, EQUAL, CARD_HEIGHT)
            | bgcolor(Color::Blue)
            | color(Color::White);
    }

    std::pair<std::string, std::string> parsed = parseCard(cardStr);
    std::string rank = parsed.first;
    std::string suit = parsed.second;
    Color cardColor = isRedSuit(cardStr) ? Color::Red : Color::White;
    std::string blank(INNER_WIDTH, ' ');

    std::string topLine = " " + padRight(rank, INNER_WIDTH - 1);
    std::string suitLine = centerStr(suit, INNER_WIDTH);
    std::string bottomLine = padLeft(rank + " ", INNER_WIDTH);

    Element content = vbox({
        text(topLine) | color(cardColor),
        text(blank),
        text(suitLine) | bold | color(cardColor),
        text(blank),
        text(bottomLine) | color(cardColor),
    });

    return content
        | border
        | size(WIDTH, EQUAL, CARD_WIDTH)
        | size(HEIGHT, EQUAL, CARD_HEIGHT)
        | bgcolor(Color::GrayDark);
}

Element renderCardRow(const std::vector<std::string>& cards, bool faceDown) {
    Elements elems;
    for (const std::string& card : cards) {
        elems.push_back(renderCard(card, faceDown));
    }
    return hbox(elems);
}

Element renderEmptySlot() {
    std::string blank(INNER_WIDTH, ' ');
    Element content = vbox({
        text(blank),
        text(blank),
        text(blank),
        text(blank),
        text(blank),
    });
    return content
        | border
        | size(WIDTH, EQUAL, CARD_WIDTH)
        | size(HEIGHT, EQUAL, CARD_HEIGHT)
        | dim;
}

}

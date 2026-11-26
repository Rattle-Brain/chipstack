#pragma once

#include "core/cards/card.hpp"
#include <string>

class CardHolder {
public:
	explicit CardHolder(std::string name) : name(std::move(name)) {}

	virtual ~CardHolder() = default;

	const std::string& getName() const { return name; }
	const Cards& getHand() const { return hand; }
	void receiveCard(const Card& card) { hand.push_back(card); }
	void clearHand() { hand.clear(); }

private:
	std::string name;
	Cards hand;
};

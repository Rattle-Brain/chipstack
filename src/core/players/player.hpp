#pragma once

#include "card_holder.hpp"

class Player : public CardHolder {
public:
	Player(std::string name, int chips)
			: CardHolder(std::move(name)), chips(chips) {}
	int getChips() const { return chips; }
	bool isBustedOut() const { return chips <= 0; }
	void receiveChips(int amount) { chips += amount; }

	bool placeBet(int amount) {
		if (amount > chips) return false;
		chips -= amount;
		return true;
	}

private:
	int chips;
};

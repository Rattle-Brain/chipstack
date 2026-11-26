#pragma once

#include "poker_player.hpp"
#include <vector>

enum class SeatState { Active, Folded, AllIn, SittingOut };

struct PokerSeat {
	PokerPlayer* player;
	SeatState state = SeatState::Active;
	int roundBet = 0;
	int totalInvested = 0;

	bool isActive() const { return state == SeatState::Active; }

	bool isInHand() const {
		return state == SeatState::Active || state == SeatState::AllIn;
	}

	int commitChips(int requested);
};

using PokerSeats = std::vector<PokerSeat>;

int countActiveSeats(const PokerSeats& seats);
int countSeatsInHand(const PokerSeats& seats);

#include "poker_seat.hpp"
#include <algorithm>

int PokerSeat::commitChips(int requested) {
	int committed = std::min(requested, player->getChips());
	player->placeBet(committed);
	roundBet += committed;
	totalInvested += committed;
	if (player->getChips() == 0) state = SeatState::AllIn;
	return committed;
}

int countActiveSeats(const PokerSeats& seats) {
	return std::count_if(seats.begin(), seats.end(), [](const PokerSeat& seat) {
		return seat.isActive();
	});
}

int countSeatsInHand(const PokerSeats& seats) {
	return std::count_if(seats.begin(), seats.end(), [](const PokerSeat& seat) {
		return seat.isInHand();
	});
}

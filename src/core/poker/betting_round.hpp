#pragma once

#include "betting_context.hpp"
#include "poker_observer.hpp"
#include "poker_seat.hpp"

class BettingRound {
public:
	BettingRound(
			PokerSeats& seats, int pot, Blinds blinds,
			PokerObserver* observer = nullptr
	);

	int execute(
			BettingRoundType roundType, const Cards& communityCards, int dealerButton,
			int startIndex
	);

private:
	void resetRoundBets();
	void postBlinds();
	void postBlind(int seatIndex, int amount, const std::string& blindName);
	void runActions(int startIndex);
	bool needsAction(int seatIndex) const;
	void takeTurn(int seatIndex);
	void reopenActionAfterRaise(int raiserIndex);
	int commit(PokerSeat& seat, int amount);
	PokerAction validateAction(
			PokerAction requested, const PokerSeat& seat
	) const;
	PokerAction applyAction(PokerSeat& seat, const PokerAction& action);
	int nextActiveIndex(int fromIndex) const;
	Position computePosition(int seatIndex) const;
	BettingContext buildContext(int seatIndex) const;

	PokerSeats& seats;
	PokerObserver& observer;
	int pot;
	Blinds blinds;
	int betToMatch = 0;
	BettingRoundType roundType = BettingRoundType::PreFlop;
	Cards communityCards;
	int dealerButton = 0;
	std::vector<bool> pendingAction;
};

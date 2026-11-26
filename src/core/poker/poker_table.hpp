#pragma once

#include "betting_round.hpp"
#include "core/table/table.hpp"
#include "hand_evaluator.hpp"

enum class PokerState {
	WaitingForPlayers,
	PreFlop,
	Flop,
	Turn,
	River,
	Showdown,
	HandComplete
};

struct Street {
	BettingRoundType round;
	PokerState requiredState;
	PokerState resultingState;
	int cardCount;
	const char* name;
	const char* announcement;
};

class PokerTable : public Table<PokerPlayer, PokerObserver> {
public:
	PokerTable(int smallBlind = 10, int bigBlind = 20);

	void playHand() override;
	bool deal();
	void dealFlop();
	void dealTurn();
	void dealRiver();
	void showdown();
	void collectCards();
	void advanceDealerButton();

	const Cards& getCommunityCards() const { return communityCards; }
	int getPot() const { return pot; }
	PokerState getState() const { return state; }

	int getDealerButton() const;

private:
	struct Contender {
		PokerPlayer* player;
		HandResult hand;
	};

	void seatPlayers();
	void dealHoleCards();
	void announceHoleCards();
	void dealStreet(const Street& street);
	void playStreet(const Street& street);
	void runBettingRound(BettingRoundType round);
	int bettingStartIndex(BettingRoundType round) const;
	bool isHandDecided() const;
	void settlePot();
	void awardPotToLastPlayer();
	std::vector<Contender> revealContenders();
	static HandResult bestHand(const std::vector<Contender>& contenders);
	static std::vector<PokerPlayer*> holdersOf(
			const HandResult& hand, const std::vector<Contender>& contenders
	);
	void payWinners(
			const std::vector<PokerPlayer*>& winners, const std::string& reason
	);

	Cards communityCards;
	PokerSeats seats;
	int dealerButton = 0;
	int pot = 0;
	Blinds blinds;
	PokerState state = PokerState::WaitingForPlayers;
};

#include "poker_table.hpp"

static const Street FLOP{
		BettingRoundType::Flop, PokerState::PreFlop, PokerState::Flop, 3, "Flop",
		"Dealing the flop..."
};
static const Street TURN{
		BettingRoundType::Turn, PokerState::Flop, PokerState::Turn, 1, "Turn",
		"Dealing the turn..."
};
static const Street RIVER{
		BettingRoundType::River, PokerState::Turn, PokerState::River, 1, "River",
		"Dealing the river..."
};

PokerTable::PokerTable(int smallBlind, int bigBlind)
		: blinds{smallBlind, bigBlind} {}

void PokerTable::playHand() {
	if (!deal()) return;
	observer->onHandStarted(*players[dealerButton], blinds.small, blinds.big);
	announceHoleCards();
	observer->onStreetStarted("Pre-Flop", communityCards);
	runBettingRound(BettingRoundType::PreFlop);
	for (const Street* street : {&FLOP, &TURN, &RIVER}) {
		if (isHandDecided()) break;
		playStreet(*street);
	}
	settlePot();
	collectCards();
	advanceDealerButton();
}

bool PokerTable::deal() {
	bool idle = state == PokerState::WaitingForPlayers
			|| state == PokerState::HandComplete;
	if (!idle) return reportError("Cannot deal: hand already in progress");
	if (players.size() < 2) {
		return reportError("Cannot deal: need at least 2 players");
	}
	dealerButton = getDealerButton();
	seatPlayers();
	dealer.shuffleDeck();
	dealHoleCards();
	state = PokerState::PreFlop;
	return true;
}

void PokerTable::seatPlayers() {
	seats.clear();
	for (PokerPlayer* player : players) {
		SeatState seatState =
				player->isBustedOut() ? SeatState::SittingOut : SeatState::Active;
		seats.push_back({player, seatState});
	}
}

void PokerTable::dealHoleCards() {
	for (int round = 0; round < 2; round++) {
		for (const PokerSeat& seat : seats) {
			if (seat.isActive()) dealer.dealTo(*seat.player);
		}
	}
}

void PokerTable::announceHoleCards() {
	for (const PokerSeat& seat : seats) {
		if (seat.isActive()) observer->onHoleCardsDealt(*seat.player);
	}
}

void PokerTable::dealFlop() {
	dealStreet(FLOP);
}

void PokerTable::dealTurn() {
	dealStreet(TURN);
}

void PokerTable::dealRiver() {
	dealStreet(RIVER);
}

void PokerTable::dealStreet(const Street& street) {
	if (state != street.requiredState) {
		reportError("Cannot deal " + std::string(street.name) + ": wrong state");
		return;
	}
	for (int i = 0; i < street.cardCount; i++) {
		communityCards.push_back(dealer.drawCard());
	}
	state = street.resultingState;
}

void PokerTable::playStreet(const Street& street) {
	dealStreet(street);
	observer->onStreetStarted(street.name, communityCards);
	observer->onAnnouncement(street.announcement);
	runBettingRound(street.round);
}

void PokerTable::runBettingRound(BettingRoundType round) {
	BettingRound bettingRound(seats, pot, blinds, observer);
	pot = bettingRound.execute(
			round, communityCards, dealerButton, bettingStartIndex(round)
	);
}

int PokerTable::bettingStartIndex(BettingRoundType round) const {
	int seatCount = seats.size();
	if (round != BettingRoundType::PreFlop) return (dealerButton + 1) % seatCount;
	if (seatCount == 2) return dealerButton;
	return (dealerButton + 3) % seatCount;
}

bool PokerTable::isHandDecided() const {
	return countSeatsInHand(seats) <= 1;
}

void PokerTable::settlePot() {
	if (isHandDecided()) {
		awardPotToLastPlayer();
	} else {
		showdown();
	}
}

void PokerTable::awardPotToLastPlayer() {
	for (const PokerSeat& seat : seats) {
		if (!seat.isInHand()) continue;
		payWinners({seat.player}, "everyone else folded");
		return;
	}
}

void PokerTable::showdown() {
	if (state != PokerState::River) {
		reportError("Cannot showdown: wrong state");
		return;
	}
	state = PokerState::Showdown;
	observer->onShowdownBegin(communityCards);
	observer->onAnnouncement("Players reveal their hands...");
	std::vector<Contender> contenders = revealContenders();
	HandResult best = bestHand(contenders);
	payWinners(holdersOf(best, contenders), best.getRankName());
	state = PokerState::HandComplete;
}

HandResult PokerTable::bestHand(const std::vector<Contender>& contenders) {
	HandResult best;
	for (const Contender& contender : contenders) {
		best = std::max(best, contender.hand);
	}
	return best;
}

std::vector<PokerPlayer*> PokerTable::holdersOf(
		const HandResult& hand, const std::vector<Contender>& contenders
) {
	std::vector<PokerPlayer*> holders;
	for (const Contender& contender : contenders) {
		if (contender.hand == hand) holders.push_back(contender.player);
	}
	return holders;
}

std::vector<PokerTable::Contender> PokerTable::revealContenders() {
	std::vector<Contender> contenders;
	for (const PokerSeat& seat : seats) {
		if (!seat.isInHand()) continue;
		Cards cards = combineCards(seat.player->getHand(), communityCards);
		HandResult hand = HandEvaluator::evaluate(cards);
		observer->onShowdownReveal(*seat.player, hand.getRankName());
		contenders.push_back({seat.player, hand});
	}
	return contenders;
}

void PokerTable::payWinners(
		const std::vector<PokerPlayer*>& winners, const std::string& reason
) {
	int share = pot / winners.size();
	for (PokerPlayer* winner : winners) {
		winner->receiveChips(share);
	}
	winners.front()->receiveChips(pot % winners.size());
	if (winners.size() == 1) {
		observer->onPotWon(*winners.front(), pot, reason);
		observer->onAnnouncement(
				winners.front()->getName() + " takes the pot (" + reason + ")"
		);
	} else {
		observer->onSplitPot({winners.begin(), winners.end()}, share, reason);
		observer->onAnnouncement("Split pot - " + std::to_string(share) + " each");
	}
	pot = 0;
}

void PokerTable::collectCards() {
	returnCardsToDealer();
	communityCards.clear();
	seats.clear();
	pot = 0;
	state = PokerState::WaitingForPlayers;
}

int PokerTable::getDealerButton() const {
	return players.empty() ? 0 : dealerButton % players.size();
}

void PokerTable::advanceDealerButton() {
	if (!players.empty()) dealerButton = (dealerButton + 1) % players.size();
}

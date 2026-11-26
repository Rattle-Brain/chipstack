#include "betting_round.hpp"
#include <algorithm>

static PokerObserver& observerOrSilent(PokerObserver* observer) {
	static PokerObserver silentObserver;
	return observer ? *observer : silentObserver;
}

BettingRound::BettingRound(
		PokerSeats& seats, int pot, Blinds blinds, PokerObserver* observer
)
		: seats(seats), observer(observerOrSilent(observer)), pot(pot),
			blinds(blinds) {}

int BettingRound::execute(
		BettingRoundType type, const Cards& cards, int button, int startIndex
) {
	roundType = type;
	communityCards = cards;
	dealerButton = button;
	resetRoundBets();
	if (roundType == BettingRoundType::PreFlop) postBlinds();
	runActions(startIndex % seats.size());
	return pot;
}

void BettingRound::resetRoundBets() {
	betToMatch = 0;
	for (PokerSeat& seat : seats) {
		seat.roundBet = 0;
	}
}

void BettingRound::postBlinds() {
	int seatCount = seats.size();
	bool headsUp = seatCount == 2;
	int smallBlindIndex = headsUp ? dealerButton : (dealerButton + 1) % seatCount;
	postBlind(smallBlindIndex, blinds.small, "small blind");
	postBlind((smallBlindIndex + 1) % seatCount, blinds.big, "big blind");
}

void BettingRound::postBlind(
		int seatIndex, int amount, const std::string& blindName
) {
	PokerSeat& seat = seats[seatIndex];
	int posted = commit(seat, amount);
	observer.onBlindPosted(*seat.player, posted, blindName, pot);
}

void BettingRound::runActions(int startIndex) {
	pendingAction.assign(seats.size(), false);
	for (size_t i = 0; i < seats.size(); i++) {
		pendingAction[i] = seats[i].isActive();
	}
	int seatIndex =
			seats[startIndex].isActive() ? startIndex : nextActiveIndex(startIndex);
	while (seatIndex >= 0 && needsAction(seatIndex)) {
		takeTurn(seatIndex);
		seatIndex = nextActiveIndex(seatIndex);
	}
}

bool BettingRound::needsAction(int seatIndex) const {
	if (!pendingAction[seatIndex] || countSeatsInHand(seats) <= 1) return false;
	bool facingBet = seats[seatIndex].roundBet < betToMatch;
	return countActiveSeats(seats) > 1 || facingBet;
}

void BettingRound::takeTurn(int seatIndex) {
	PokerSeat& seat = seats[seatIndex];
	int previousBet = betToMatch;
	BettingContext context = buildContext(seatIndex);
	observer.onBeforeDecision(*seat.player);
	PokerAction requested = seat.player->decidePokerAction(context);
	PokerAction applied = applyAction(seat, validateAction(requested, seat));
	observer.onPlayerAction(*seat.player, applied, pot);
	pendingAction[seatIndex] = false;
	if (betToMatch > previousBet) reopenActionAfterRaise(seatIndex);
}

void BettingRound::reopenActionAfterRaise(int raiserIndex) {
	for (size_t i = 0; i < seats.size(); i++) {
		if (static_cast<int>(i) != raiserIndex && seats[i].isActive()) {
			pendingAction[i] = true;
		}
	}
}

int BettingRound::commit(PokerSeat& seat, int amount) {
	int committed = seat.commitChips(amount);
	pot += committed;
	betToMatch = std::max(betToMatch, seat.roundBet);
	return committed;
}

PokerAction BettingRound::validateAction(
		PokerAction requested, const PokerSeat& seat
) const {
	int costToCall = betToMatch - seat.roundBet;
	int stack = seat.player->getChips();
	switch (requested.type) {
		case ActionType::Check:
			return costToCall > 0 ? PokerAction::fold() : requested;
		case ActionType::Call:
			if (costToCall == 0) return PokerAction::check();
			return costToCall >= stack ? PokerAction::allIn() : requested;
		case ActionType::Raise: {
			int raiseTo = std::max(requested.amount, betToMatch + blinds.big);
			if (raiseTo >= stack + seat.roundBet) return PokerAction::allIn();
			return PokerAction::raise(raiseTo);
		}
		default:
			return requested;
	}
}

PokerAction BettingRound::applyAction(
		PokerSeat& seat, const PokerAction& action
) {
	switch (action.type) {
		case ActionType::Fold:
			seat.state = SeatState::Folded;
			return action;
		case ActionType::Check:
			return action;
		case ActionType::Call:
			return {ActionType::Call, commit(seat, betToMatch - seat.roundBet)};
		case ActionType::Raise:
			commit(seat, action.amount - seat.roundBet);
			return {ActionType::Raise, seat.roundBet};
		case ActionType::AllIn:
			commit(seat, seat.player->getChips());
			return {ActionType::AllIn, seat.roundBet};
	}
	return action;
}

int BettingRound::nextActiveIndex(int fromIndex) const {
	int seatCount = seats.size();
	for (int offset = 1; offset <= seatCount; offset++) {
		int index = (fromIndex + offset) % seatCount;
		if (seats[index].isActive()) return index;
	}
	return -1;
}

Position BettingRound::computePosition(int seatIndex) const {
	int seatCount = seats.size();
	int distance = (seatIndex - dealerButton + seatCount) % seatCount;
	if (distance == 0) return Position::Dealer;
	if (distance <= 2) return Position::EarlyBlind;
	int playablePositions = seatCount - 2;
	int third = std::max(1, playablePositions / 3);
	int positionInOrder = distance - 3;
	if (positionInOrder < third) return Position::Early;
	if (positionInOrder < third * 2) return Position::Middle;
	return Position::Late;
}

BettingContext BettingRound::buildContext(int seatIndex) const {
	const PokerSeat& seat = seats[seatIndex];
	BettingContext context;
	context.roundType = roundType;
	context.communityCards = communityCards;
	context.pot = pot;
	context.currentBet = betToMatch;
	context.playerCurrentBet = seat.roundBet;
	context.costToCall = betToMatch - seat.roundBet;
	context.playerChips = seat.player->getChips();
	context.numActivePlayers = countSeatsInHand(seats);
	context.numPlayersToAct = countActiveSeats(seats) - 1;
	context.minRaise = blinds.big;
	context.bigBlind = blinds.big;
	context.dealerButton = dealerButton;
	context.playerSeatIndex = seatIndex;
	context.totalPlayers = seats.size();
	context.position = computePosition(seatIndex);
	return context;
}

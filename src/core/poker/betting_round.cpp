#include "betting_round.hpp"

BettingRound::BettingRound(std::vector<PlayerSeat>& seats, int pot, int bigBlind,
                           IGameObserver* observer,
                           std::function<void(IPlayer*)> preActionCallback)
    : seats(seats), currentBet(0), pot(pot), bigBlind(bigBlind),
      observer_(observer), preActionCallback_(preActionCallback) {}

int BettingRound::nextActiveIndex(int from) const {
    int n = seats.size();
    for (int i = 1; i <= n; i++) {
        int idx = (from + i) % n;
        if (seats[idx].state == PlayerHandState::Active) {
            return idx;
        }
    }
    return -1;
}

int BettingRound::countActivePlayers() const {
    int count = 0;
    for (size_t i = 0; i < seats.size(); i++) {
        if (seats[i].state == PlayerHandState::Active) count++;
    }
    return count;
}

int BettingRound::countNonFoldedPlayers() const {
    int count = 0;
    for (size_t i = 0; i < seats.size(); i++) {
        if (seats[i].state != PlayerHandState::Folded &&
            seats[i].state != PlayerHandState::SittingOut) {
            count++;
        }
    }
    return count;
}

Position BettingRound::computePosition(int seatIndex, int dealerButton) const {
    int n = seats.size();
    int distance = (seatIndex - dealerButton + n) % n;

    if (distance == 0) return Position::Dealer;
    if (distance == 1) return Position::EarlyBlind;
    if (distance == 2) return Position::EarlyBlind;

    int playablePositions = n - 2;
    if (playablePositions <= 0) return Position::Early;

    int posInSequence = distance - 3;
    int third = playablePositions / 3;
    if (third == 0) third = 1;

    if (posInSequence < third) return Position::Early;
    if (posInSequence < third * 2) return Position::Middle;
    return Position::Late;
}

BettingContext BettingRound::buildContext(int seatIndex, BettingRoundType roundType,
                                         const std::vector<Card>& communityCards,
                                         int dealerButton, int playersToAct) const {
    BettingContext ctx;
    ctx.roundType = roundType;
    ctx.communityCards = communityCards;
    ctx.pot = pot;
    ctx.currentBet = currentBet;
    ctx.playerCurrentBet = seats[seatIndex].currentBet;
    ctx.costToCall = currentBet - seats[seatIndex].currentBet;
    ctx.playerChips = seats[seatIndex].player->getChips();
    ctx.numActivePlayers = countNonFoldedPlayers();
    ctx.numPlayersToAct = playersToAct;
    ctx.minRaise = bigBlind;
    ctx.bigBlind = bigBlind;
    ctx.dealerButton = dealerButton;
    ctx.playerSeatIndex = seatIndex;
    ctx.totalPlayers = seats.size();
    ctx.position = computePosition(seatIndex, dealerButton);
    return ctx;
}

PokerAction BettingRound::validateAction(PokerAction requested, const PlayerSeat& seat) const {
    int costToCall = currentBet - seat.currentBet;
    int chips = seat.player->getChips();

    switch (requested.type) {
        case ActionType::Check:
            if (costToCall > 0) return PokerAction::fold();
            break;
        case ActionType::Call:
            if (costToCall == 0) return PokerAction::check();
            if (costToCall >= chips) return PokerAction::allIn();
            break;
        case ActionType::Raise:
            if (requested.amount < currentBet + bigBlind) {
                requested.amount = currentBet + bigBlind;
            }
            if (requested.amount >= chips + seat.currentBet) {
                return PokerAction::allIn();
            }
            break;
        case ActionType::AllIn:
        case ActionType::Fold:
            break;
    }
    return requested;
}

void BettingRound::applyAction(PlayerSeat& seat, PokerAction action) {
    switch (action.type) {
        case ActionType::Fold:
            seat.state = PlayerHandState::Folded;
            if (observer_) observer_->onPlayerAction(seat.player->getName(), "Fold", 0, pot, seat.player->getChips());
            break;
        case ActionType::Check:
            if (observer_) observer_->onPlayerAction(seat.player->getName(), "Check", 0, pot, seat.player->getChips());
            break;
        case ActionType::Call: {
            int costToCall = currentBet - seat.currentBet;
            seat.player->placeBet(costToCall);
            seat.currentBet = currentBet;
            seat.totalInvested += costToCall;
            pot += costToCall;
            if (observer_) observer_->onPlayerAction(seat.player->getName(), "Call", costToCall, pot, seat.player->getChips());
            break;
        }
        case ActionType::Raise: {
            int totalForRound = action.amount;
            int additional = totalForRound - seat.currentBet;
            seat.player->placeBet(additional);
            seat.currentBet = totalForRound;
            seat.totalInvested += additional;
            pot += additional;
            currentBet = totalForRound;
            if (observer_) observer_->onPlayerAction(seat.player->getName(), "Raise", totalForRound, pot, seat.player->getChips());
            break;
        }
        case ActionType::AllIn: {
            int chips = seat.player->getChips();
            seat.player->placeBet(chips);
            int totalForRound = seat.currentBet + chips;
            seat.currentBet = totalForRound;
            seat.totalInvested += chips;
            pot += chips;
            if (totalForRound > currentBet) {
                currentBet = totalForRound;
            }
            seat.state = PlayerHandState::AllIn;
            if (observer_) observer_->onPlayerAction(seat.player->getName(), "All-In", totalForRound, pot, seat.player->getChips());
            break;
        }
    }
}

int BettingRound::execute(BettingRoundType roundType,
                          const std::vector<Card>& communityCards,
                          int dealerButton, int startIndex) {
    int n = seats.size();

    for (size_t i = 0; i < seats.size(); i++) {
        seats[i].currentBet = 0;
    }

    currentBet = 0;

    if (roundType == BettingRoundType::PreFlop) {
        int sbIdx = (dealerButton + 1) % n;
        int bbIdx = (dealerButton + 2) % n;
        if (n == 2) {
            sbIdx = dealerButton;
            bbIdx = (dealerButton + 1) % n;
        }

        int sbAmount = bigBlind / 2;
        int bbAmount = bigBlind;

        if (seats[sbIdx].player->getChips() <= sbAmount) {
            sbAmount = seats[sbIdx].player->getChips();
        }
        seats[sbIdx].player->placeBet(sbAmount);
        seats[sbIdx].currentBet = sbAmount;
        seats[sbIdx].totalInvested += sbAmount;
        pot += sbAmount;

        if (seats[bbIdx].player->getChips() <= bbAmount) {
            bbAmount = seats[bbIdx].player->getChips();
        }
        seats[bbIdx].player->placeBet(bbAmount);
        seats[bbIdx].currentBet = bbAmount;
        seats[bbIdx].totalInvested += bbAmount;
        pot += bbAmount;

        currentBet = bbAmount;

        if (observer_) {
            observer_->onBlindsPosted(seats[sbIdx].player->getName(), sbAmount, "small blind", pot, seats[sbIdx].player->getChips());
            observer_->onBlindsPosted(seats[bbIdx].player->getName(), bbAmount, "big blind", pot, seats[bbIdx].player->getChips());
        }
    }

    if (countActivePlayers() <= 1) return pot;

    std::vector<bool> needsToAct(n, false);
    for (int i = 0; i < n; i++) {
        if (seats[i].state == PlayerHandState::Active) {
            needsToAct[i] = true;
        }
    }

    int actingIdx = startIndex;
    int safetyLimit = n * (n + 2);
    int iterations = 0;

    while (iterations++ < safetyLimit) {
        if (seats[actingIdx].state != PlayerHandState::Active) {
            int next = nextActiveIndex(actingIdx);
            if (next < 0) break;
            actingIdx = next;
            continue;
        }

        if (countActivePlayers() <= 1) break;
        if (!needsToAct[actingIdx]) break;

        int playersAfter = 0;
        int tempIdx = actingIdx;
        for (int i = 0; i < n - 1; i++) {
            tempIdx = (tempIdx + 1) % n;
            if (seats[tempIdx].state == PlayerHandState::Active) {
                playersAfter++;
            }
        }

        BettingContext ctx = buildContext(actingIdx, roundType, communityCards,
                                         dealerButton, playersAfter);
        int prevBet = currentBet;

        if (preActionCallback_) preActionCallback_(seats[actingIdx].player);

        PokerAction action = seats[actingIdx].player->decideAction(ctx);
        action = validateAction(action, seats[actingIdx]);
        applyAction(seats[actingIdx], action);
        needsToAct[actingIdx] = false;

        bool raisedBet = (currentBet > prevBet);
        if (raisedBet) {
            for (int i = 0; i < n; i++) {
                if (i != actingIdx && seats[i].state == PlayerHandState::Active) {
                    needsToAct[i] = true;
                }
            }
        }

        int nextIdx = nextActiveIndex(actingIdx);
        if (nextIdx < 0) break;
        actingIdx = nextIdx;
    }

    return pot;
}

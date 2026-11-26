#include "poker_table.hpp"

PokerTable::PokerTable(int smallBlind, int bigBlind)
    : dealerButton(0), pot(0), smallBlind(smallBlind), bigBlind(bigBlind),
      state(PokerState::WaitingForPlayers) {}

PokerTable::~PokerTable() {}

void PokerTable::initSeats() {
    seats.clear();
    for (size_t i = 0; i < players.size(); i++) {
        PlayerSeat seat;
        seat.player = players[i];
        seat.state = players[i]->isBustedOut() ? PlayerHandState::SittingOut : PlayerHandState::Active;
        seat.currentBet = 0;
        seat.totalInvested = 0;
        seats.push_back(seat);
    }
}

void PokerTable::deal() {
    if (state != PokerState::WaitingForPlayers && state != PokerState::HandComplete) {
        if (observer_) observer_->onError("Cannot deal: hand already in progress");
        return;
    }
    if (players.size() < 2) {
        if (observer_) observer_->onError("Cannot deal: need at least 2 players");
        return;
    }
    initSeats();
    crupier.shuffleDeck();
    for (int round = 0; round < 2; round++) {
        for (size_t i = 0; i < players.size(); i++) {
            if (seats[i].state == PlayerHandState::Active) {
                crupier.dealCard(*players[i]);
            }
        }
    }
    state = PokerState::PreFlop;
}

void PokerTable::collectCards() {
    for (size_t i = 0; i < players.size(); i++) {
        players[i]->clearHand();
    }
    crupier.clearHand();
    communityCards.clear();
    seats.clear();
    crupier.resetDeck();
    resetPot();
    state = PokerState::WaitingForPlayers;
}

void PokerTable::dealFlop() {
    if (state != PokerState::PreFlop) {
        if (observer_) observer_->onError("Cannot deal flop: wrong state");
        return;
    }
    for (int i = 0; i < 3; i++) {
        communityCards.push_back(crupier.pullCard());
    }
    state = PokerState::Flop;
}

void PokerTable::dealTurn() {
    if (state != PokerState::Flop) {
        if (observer_) observer_->onError("Cannot deal turn: wrong state");
        return;
    }
    communityCards.push_back(crupier.pullCard());
    state = PokerState::Turn;
}

void PokerTable::dealRiver() {
    if (state != PokerState::Turn) {
        if (observer_) observer_->onError("Cannot deal river: wrong state");
        return;
    }
    communityCards.push_back(crupier.pullCard());
    state = PokerState::River;
}

int PokerTable::getBettingStartIndex(BettingRoundType roundType) const {
    int n = seats.size();
    if (roundType == BettingRoundType::PreFlop) {
        if (n == 2) return dealerButton;
        return (dealerButton + 3) % n;
    }
    return (dealerButton + 1) % n;
}

void PokerTable::runBettingRound(BettingRoundType roundType) {
    BettingRound round(seats, pot, bigBlind, observer_, preActionCallback);
    int startIdx = getBettingStartIndex(roundType);
    pot = round.execute(roundType, communityCards, dealerButton, startIdx);
}

void PokerTable::awardPotToLastPlayer() {
    for (size_t i = 0; i < seats.size(); i++) {
        if (seats[i].state != PlayerHandState::Folded &&
            seats[i].state != PlayerHandState::SittingOut) {
            if (observer_) observer_->onPotWon(seats[i].player->getName(), pot, "everyone else folded");
            seats[i].player->receiveChips(pot);
            if (postPhaseCallback) postPhaseCallback(seats[i].player->getName() + " takes the pot");
            resetPot();
            return;
        }
    }
}

void PokerTable::playHand() {
    if (observer_) observer_->onHandStarted(players[dealerButton]->getName(), smallBlind, bigBlind);

    deal();

    for (size_t i = 0; i < players.size(); i++) {
        if (seats[i].state == PlayerHandState::Active) {
            const std::vector<Card>& h = players[i]->getHand();
            if (observer_) observer_->onPlayerCards(players[i]->getName(),
                h[0].toString(), h[1].toString());
        }
    }

    if (observer_) observer_->onPhaseChanged("Pre-Flop", "");
    runBettingRound(BettingRoundType::PreFlop);

    BettingRound checker(seats, pot, bigBlind);
    if (checker.countNonFoldedPlayers() <= 1) {
        awardPotToLastPlayer();
        collectCards();
        advanceDealerButton();
        return;
    }

    dealFlop();
    if (observer_) observer_->onPhaseChanged("Flop", showCommunityCards());
    if (postPhaseCallback) postPhaseCallback("Dealing the flop...");
    runBettingRound(BettingRoundType::Flop);

    BettingRound checker2(seats, pot, bigBlind);
    if (checker2.countNonFoldedPlayers() <= 1) {
        awardPotToLastPlayer();
        collectCards();
        advanceDealerButton();
        return;
    }

    dealTurn();
    if (observer_) observer_->onPhaseChanged("Turn", showCommunityCards());
    if (postPhaseCallback) postPhaseCallback("Dealing the turn...");
    runBettingRound(BettingRoundType::Turn);

    BettingRound checker3(seats, pot, bigBlind);
    if (checker3.countNonFoldedPlayers() <= 1) {
        awardPotToLastPlayer();
        collectCards();
        advanceDealerButton();
        return;
    }

    dealRiver();
    if (observer_) observer_->onPhaseChanged("River", showCommunityCards());
    if (postPhaseCallback) postPhaseCallback("Dealing the river...");
    runBettingRound(BettingRoundType::River);

    BettingRound checker4(seats, pot, bigBlind);
    if (checker4.countNonFoldedPlayers() <= 1) {
        awardPotToLastPlayer();
    } else {
        showdown();
    }

    collectCards();
    advanceDealerButton();
}

void PokerTable::showdown() {
    if (state != PokerState::River) {
        if (observer_) observer_->onError("Cannot showdown: wrong state");
        return;
    }
    state = PokerState::Showdown;

    if (observer_) observer_->onShowdownBegin(showCommunityCards());
    if (postPhaseCallback) postPhaseCallback("Players reveal their hands...");

    std::vector<int> eligible;
    std::vector<HandResult> results;

    for (size_t i = 0; i < seats.size(); i++) {
        if (seats[i].state == PlayerHandState::Folded ||
            seats[i].state == PlayerHandState::SittingOut) {
            results.push_back(HandResult());
            continue;
        }

        eligible.push_back(i);

        std::vector<Card> allCards;
        const std::vector<Card>& hand = players[i]->getHand();
        for (size_t j = 0; j < hand.size(); j++) {
            allCards.push_back(hand[j]);
        }
        for (size_t j = 0; j < communityCards.size(); j++) {
            allCards.push_back(communityCards[j]);
        }
        HandResult result = HandEvaluator::evaluate(allCards);
        results.push_back(result);

        std::string cards;
        for (size_t j = 0; j < hand.size(); j++) {
            if (j > 0) cards += " ";
            cards += hand[j].toString();
        }
        if (observer_) observer_->onShowdownReveal(players[i]->getName(), cards, result.getRankName());
    }

    if (eligible.empty()) return;

    int bestIdx = eligible[0];
    std::vector<int> winners;
    winners.push_back(bestIdx);

    for (size_t i = 1; i < eligible.size(); i++) {
        int idx = eligible[i];
        if (results[idx] > results[bestIdx]) {
            bestIdx = idx;
            winners.clear();
            winners.push_back(idx);
        } else if (results[idx] == results[bestIdx]) {
            winners.push_back(idx);
        }
    }

    if (winners.size() == 1) {
        if (observer_) observer_->onPotWon(players[bestIdx]->getName(), pot, results[bestIdx].getRankName());
        players[bestIdx]->receiveChips(pot);
        if (postPhaseCallback) postPhaseCallback(players[bestIdx]->getName() + " takes the pot with " + results[bestIdx].getRankName());
    } else {
        int share = pot / (int)winners.size();
        std::vector<std::string> winnerNames;
        for (size_t i = 0; i < winners.size(); i++) {
            winnerNames.push_back(players[winners[i]]->getName());
            players[winners[i]]->receiveChips(share);
        }
        if (observer_) observer_->onSplitPot(winnerNames, share, results[bestIdx].getRankName());
        if (postPhaseCallback) postPhaseCallback("Split pot - " + std::to_string(share) + " each");
    }

    resetPot();
    state = PokerState::HandComplete;
}

const std::vector<Card>& PokerTable::getCommunityCards() const {
    return communityCards;
}

std::string PokerTable::showCommunityCards() {
    std::string result = "Community: ";
    for (size_t i = 0; i < communityCards.size(); i++) {
        result += communityCards[i].toString();
        if (i < communityCards.size() - 1) result += " ";
    }
    return result;
}

int PokerTable::getDealerButton() const {
    return dealerButton;
}

void PokerTable::advanceDealerButton() {
    if (!players.empty()) {
        dealerButton = (dealerButton + 1) % players.size();
    }
}

int PokerTable::getPot() const {
    return pot;
}

void PokerTable::addToPot(int amount) {
    pot += amount;
}

void PokerTable::resetPot() {
    pot = 0;
}

PokerState PokerTable::getState() const {
    return state;
}

const char* PokerTable::getStateName() const {
    switch (state) {
        case PokerState::WaitingForPlayers: return "WaitingForPlayers";
        case PokerState::PreFlop: return "PreFlop";
        case PokerState::Flop: return "Flop";
        case PokerState::Turn: return "Turn";
        case PokerState::River: return "River";
        case PokerState::Showdown: return "Showdown";
        case PokerState::HandComplete: return "HandComplete";
    }
    return "Unknown";
}

int PokerTable::getSmallBlind() const {
    return smallBlind;
}

int PokerTable::getBigBlind() const {
    return bigBlind;
}

const std::vector<PlayerSeat>& PokerTable::getSeats() const {
    return seats;
}

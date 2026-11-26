#include "blackjack_table.hpp"

BlackjackTable::BlackjackTable() : state(BlackjackState::WaitingForPlayers), currentPlayerIndex(0) {}

BlackjackTable::~BlackjackTable() {}

int BlackjackTable::getHandValue(const IPlayer& player) const {
  const std::vector<Card>& hand = player.getHand();
  int total = 0;
  int aces = 0;
  for (size_t i = 0; i < hand.size(); i++) {
    int rank = hand[i].getRank();
    if (rank == 14) {
      total += 11;
      aces++;
    } else if (rank >= 11) {
      total += 10;
    } else {
      total += rank;
    }
  }
  while (total > 21 && aces > 0) {
    total -= 10;
    aces--;
  }
  return total;
}

bool BlackjackTable::isBust(const IPlayer& player) const {
  return getHandValue(player) > 21;
}

bool BlackjackTable::isBlackjack(const IPlayer& player) const {
  return player.getHand().size() == 2 && getHandValue(player) == 21;
}

bool BlackjackTable::placeBet(IPlayer& player, int amount) {
  if (!player.placeBet(amount)) {
    if (observer_) observer_->onError(player.getName() + " cannot bet " +
        std::to_string(amount) + " (has " + std::to_string(player.getChips()) + " chips)");
    return false;
  }
  bets[&player] = amount;
  return true;
}

void BlackjackTable::deal() {
  if (state != BlackjackState::WaitingForPlayers && state != BlackjackState::HandComplete) {
    if (observer_) observer_->onError("Cannot deal: hand already in progress");
    return;
  }
  if (players.empty()) {
    if (observer_) observer_->onError("Cannot deal: no players at the table");
    return;
  }
  crupier.shuffleDeck();
  for (int round = 0; round < 2; round++) {
    for (size_t i = 0; i < players.size(); i++) {
      crupier.dealCard(*players[i]);
    }
    crupier.dealCard(crupier);
  }
  currentPlayerIndex = 0;
  state = BlackjackState::PlayerTurns;

  for (size_t i = 0; i < players.size(); i++) {
    if (observer_) observer_->onPlayerDealt(players[i]->getName(),
        showCards(*players[i]), getHandValue(*players[i]));
  }
  const std::vector<Card>& crupierHand = crupier.getHand();
  if (observer_) observer_->onDealerDealt(crupierHand[0].toString());
}

void BlackjackTable::collectCards() {
  for (size_t i = 0; i < players.size(); i++) {
    players[i]->clearHand();
  }
  crupier.clearHand();
  crupier.resetDeck();
  currentPlayerIndex = 0;
  bets.clear();
  state = BlackjackState::WaitingForPlayers;
}

void BlackjackTable::hitPlayer(IPlayer& player) {
  if (state != BlackjackState::PlayerTurns) {
    if (observer_) observer_->onError("Cannot hit: not in player turns");
    return;
  }
  if (&player != players[currentPlayerIndex]) {
    if (observer_) observer_->onError("Cannot hit: not " + player.getName() + "'s turn");
    return;
  }
  crupier.dealCard(player);
  int value = getHandValue(player);
  if (observer_) observer_->onPlayerHit(player.getName(), showCards(player), value);
  if (value > 21) {
    if (observer_) observer_->onPlayerBust(player.getName());
    advancePlayer();
  }
}

void BlackjackTable::standPlayer() {
  if (state != BlackjackState::PlayerTurns) {
    if (observer_) observer_->onError("Cannot stand: not in player turns");
    return;
  }
  if (observer_) observer_->onPlayerStand(players[currentPlayerIndex]->getName(),
      getHandValue(*players[currentPlayerIndex]));
  advancePlayer();
}

bool BlackjackTable::hasSoftAce(const IPlayer& player) const {
    const std::vector<Card>& hand = player.getHand();
    int total = 0;
    int aces = 0;
    for (size_t i = 0; i < hand.size(); i++) {
        int rank = hand[i].getRank();
        if (rank == 14) {
            total += 11;
            aces++;
        } else if (rank >= 11) {
            total += 10;
        } else {
            total += rank;
        }
    }
    while (total > 21 && aces > 0) {
        total -= 10;
        aces--;
    }
    return aces > 0;
}

int BlackjackTable::getDealerUpcard() const {
    const std::vector<Card>& hand = crupier.getHand();
    if (hand.empty()) return 0;
    int rank = hand[0].getRank();
    if (rank == 14) return 11;
    if (rank >= 11) return 10;
    return rank;
}

BlackjackContext BlackjackTable::buildContext(IPlayer& player) const {
    BlackjackContext ctx;
    ctx.handValue = getHandValue(player);
    ctx.numCards = player.getHand().size();
    ctx.hasSoftAce = hasSoftAce(player);
    ctx.dealerUpcard = getDealerUpcard();
    std::map<IPlayer*, int>::const_iterator it = bets.find(&player);
    ctx.bet = (it != bets.end()) ? it->second : 0;
    ctx.playerChips = player.getChips();
    ctx.canDoubleDown = (ctx.numCards == 2 && ctx.playerChips >= ctx.bet);
    return ctx;
}

void BlackjackTable::playHand(int minBet, int maxBet) {
    if (players.empty()) {
        if (observer_) observer_->onError("Cannot play: no players at the table");
        return;
    }

    for (size_t i = 0; i < players.size(); i++) {
        BlackjackBetContext betCtx;
        betCtx.playerChips = players[i]->getChips();
        betCtx.minBet = minBet;
        betCtx.maxBet = maxBet;

        if (betCtx.playerChips < minBet) {
            betCtx.minBet = betCtx.playerChips;
        }
        if (betCtx.maxBet > betCtx.playerChips) {
            betCtx.maxBet = betCtx.playerChips;
        }

        if (preActionCallback) preActionCallback(players[i]);

        int betAmount = players[i]->decideBlackjackBet(betCtx);
        if (betAmount < betCtx.minBet) betAmount = betCtx.minBet;
        if (betAmount > betCtx.maxBet) betAmount = betCtx.maxBet;

        placeBet(*players[i], betAmount);
        if (observer_) observer_->onBetPlaced(players[i]->getName(), betAmount);
    }

    if (observer_) observer_->onDealingStarted();
    deal();

    while (state == BlackjackState::PlayerTurns) {
        IPlayer* current = getCurrentPlayer();
        if (!current) break;

        while (state == BlackjackState::PlayerTurns &&
               getCurrentPlayer() == current) {
            int value = getHandValue(*current);
            if (value >= 21) {
                if (value == 21) {
                    standPlayer();
                }
                break;
            }

            if (preActionCallback) preActionCallback(current);

            BlackjackContext ctx = buildContext(*current);
            BlackjackAction action = current->decideBlackjackPlay(ctx);

            switch (action.type) {
                case BlackjackActionType::Hit:
                    hitPlayer(*current);
                    break;
                case BlackjackActionType::Stand:
                    standPlayer();
                    break;
                case BlackjackActionType::DoubleDown: {
                    std::map<IPlayer*, int>::iterator it = bets.find(current);
                    if (it != bets.end() && ctx.canDoubleDown) {
                        int extraBet = it->second;
                        if (current->placeBet(extraBet)) {
                            it->second *= 2;
                            if (observer_) observer_->onPlayerDoubleDown(current->getName(), it->second);
                        }
                    }
                    hitPlayer(*current);
                    if (state == BlackjackState::PlayerTurns &&
                        getCurrentPlayer() == current && !isBust(*current)) {
                        standPlayer();
                    }
                    break;
                }
            }
        }
    }

    playCrupierTurn();
    collectCards();
}

void BlackjackTable::advancePlayer() {
  currentPlayerIndex++;
  if (currentPlayerIndex >= (int)players.size()) {
    state = BlackjackState::CrupierTurn;
  }
}

void BlackjackTable::playCrupierTurn() {
  if (state != BlackjackState::CrupierTurn) {
    if (observer_) observer_->onError("Cannot play crupier turn: wrong state");
    return;
  }

  int value = getHandValue(crupier);
  if (observer_) observer_->onDealerReveals(showCards(crupier), value);
  if (postPhaseCallback) postPhaseCallback("Dealer reveals hole card...");

  while (value < 17) {
    crupier.dealCard(crupier);
    value = getHandValue(crupier);
    if (observer_) observer_->onDealerHits(showCards(crupier), value);
    if (postPhaseCallback) postPhaseCallback("Dealer hits...");
  }

  if (value > 21) {
    if (observer_) observer_->onDealerBust();
    if (postPhaseCallback) postPhaseCallback("Dealer busts!");
  } else {
    if (observer_) observer_->onDealerStand(value);
    if (postPhaseCallback) postPhaseCallback("Dealer stands at " + std::to_string(value));
  }

  resolve();
}

void BlackjackTable::resolve() {
  state = BlackjackState::Resolving;
  int crupierValue = getHandValue(crupier);
  bool crupierBust = crupierValue > 21;
  bool crupierBJ = isBlackjack(crupier);

  for (size_t i = 0; i < players.size(); i++) {
    int playerValue = getHandValue(*players[i]);
    int bet = 0;
    std::map<IPlayer*, int>::iterator it = bets.find(players[i]);
    if (it != bets.end()) bet = it->second;

    if (playerValue > 21) {
      if (observer_) observer_->onBlackjackResult(players[i]->getName(), "BUST", -bet);
    } else if (isBlackjack(*players[i]) && !crupierBJ) {
      int winnings = bet * 3 / 2;
      players[i]->receiveChips(bet + winnings);
      if (observer_) observer_->onBlackjackResult(players[i]->getName(), "BLACKJACK", winnings);
    } else if (crupierBust || playerValue > crupierValue) {
      players[i]->receiveChips(bet * 2);
      if (observer_) observer_->onBlackjackResult(players[i]->getName(), "WIN", bet);
    } else if (playerValue == crupierValue) {
      players[i]->receiveChips(bet);
      if (observer_) observer_->onBlackjackResult(players[i]->getName(), "PUSH", 0);
    } else {
      if (observer_) observer_->onBlackjackResult(players[i]->getName(), "LOSE", -bet);
    }
  }

  if (postPhaseCallback) postPhaseCallback("Settling bets...");
  state = BlackjackState::HandComplete;
}

BlackjackState BlackjackTable::getState() const {
  return state;
}

const char* BlackjackTable::getStateName() const {
  switch (state) {
    case BlackjackState::WaitingForPlayers: return "WaitingForPlayers";
    case BlackjackState::PlayerTurns: return "PlayerTurns";
    case BlackjackState::CrupierTurn: return "CrupierTurn";
    case BlackjackState::Resolving: return "Resolving";
    case BlackjackState::HandComplete: return "HandComplete";
  }
  return "Unknown";
}

IPlayer* BlackjackTable::getCurrentPlayer() const {
  if (state != BlackjackState::PlayerTurns || currentPlayerIndex >= (int)players.size()) {
    return nullptr;
  }
  return players[currentPlayerIndex];
}

std::string BlackjackTable::showCards(const IPlayer& player) {
  const std::vector<Card>& hand = player.getHand();
  std::string result;
  for (size_t i = 0; i < hand.size(); i++) {
    if (i > 0) result += " ";
    result += hand[i].toString();
  }
  return result;
}

const std::map<IPlayer*, int>& BlackjackTable::getBets() const {
  return bets;
}

const Crupier& BlackjackTable::getCrupier() const {
  return crupier;
}

int BlackjackTable::getCurrentPlayerIndex() const {
  return currentPlayerIndex;
}

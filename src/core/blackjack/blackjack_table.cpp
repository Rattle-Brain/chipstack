#include "blackjack_table.hpp"
#include "blackjack_rules.hpp"

using namespace BlackjackRules;

BlackjackTable::BlackjackTable(int minBet, int maxBet)
		: minBet(minBet), maxBet(std::max(minBet, maxBet)) {}

void BlackjackTable::playHand() {
	if (players.empty()) {
		reportError("Cannot play: no players at the table");
		return;
	}
	collectBets();
	observer->onDealingStarted();
	deal();
	playPlayerTurns();
	playDealerTurn();
	settleBets();
	collectCards();
}

void BlackjackTable::collectBets() {
	for (BlackjackPlayer* player : players) {
		collectBet(*player);
	}
}

void BlackjackTable::collectBet(BlackjackPlayer& player) {
	BlackjackBetContext context = buildBetContext(player);
	observer->onBeforeDecision(player);
	int requested = player.decideBlackjackBet(context);
	int bet = std::clamp(requested, context.minBet, context.maxBet);
	player.placeBet(bet);
	bets[&player] = bet;
	observer->onBetPlaced(player, bet);
}

BlackjackBetContext BlackjackTable::buildBetContext(
		const Player& player
) const {
	int chips = player.getChips();
	return {chips, std::min(minBet, chips), std::min(maxBet, chips)};
}

void BlackjackTable::deal() {
	dealer.shuffleDeck();
	for (int round = 0; round < 2; round++) {
		for (BlackjackPlayer* player : players) {
			dealer.dealTo(*player);
		}
		dealer.dealTo(dealer);
	}
	currentPlayerIndex = 0;
	state = BlackjackState::PlayerTurns;
	announceDeal();
}

void BlackjackTable::announceDeal() {
	for (BlackjackPlayer* player : players) {
		observer->onPlayerDealt(*player, handTotal(player->getHand()));
	}
	observer->onDealerDealt(dealer);
}

void BlackjackTable::playPlayerTurns() {
	while (BlackjackPlayer* player = getCurrentPlayer()) {
		playTurn(*player);
	}
}

void BlackjackTable::playTurn(BlackjackPlayer& player) {
	while (getCurrentPlayer() == &player) {
		if (handTotal(player.getHand()) == BLACKJACK) {
			standCurrentPlayer();
			return;
		}
		observer->onBeforeDecision(player);
		BlackjackContext context = buildContext(player);
		BlackjackAction action = player.decideBlackjackAction(context);
		applyAction(player, action, context.canDoubleDown);
	}
}

void BlackjackTable::applyAction(
		BlackjackPlayer& player, BlackjackAction action, bool canDoubleDown
) {
	switch (action.type) {
		case BlackjackActionType::Hit:
			hitCurrentPlayer();
			break;
		case BlackjackActionType::Stand:
			standCurrentPlayer();
			break;
		case BlackjackActionType::DoubleDown:
			doubleDown(player, canDoubleDown);
			break;
	}
}

void BlackjackTable::doubleDown(BlackjackPlayer& player, bool allowed) {
	int& bet = bets[&player];
	if (allowed && player.placeBet(bet)) {
		bet *= 2;
		observer->onPlayerDoubleDown(player, bet);
	}
	hitCurrentPlayer();
	if (getCurrentPlayer() == &player) standCurrentPlayer();
}

void BlackjackTable::hitCurrentPlayer() {
	BlackjackPlayer& player = *players[currentPlayerIndex];
	dealer.dealTo(player);
	int total = handTotal(player.getHand());
	observer->onPlayerHit(player, total);
	if (total <= BLACKJACK) return;
	observer->onPlayerBust(player);
	advancePlayer();
}

void BlackjackTable::standCurrentPlayer() {
	BlackjackPlayer& player = *players[currentPlayerIndex];
	observer->onPlayerStand(player, handTotal(player.getHand()));
	advancePlayer();
}

void BlackjackTable::advancePlayer() {
	if (++currentPlayerIndex >= players.size()) {
		state = BlackjackState::DealerTurn;
	}
}

BlackjackPlayer* BlackjackTable::getCurrentPlayer() const {
	bool playersTurn = state == BlackjackState::PlayerTurns
			&& currentPlayerIndex < players.size();
	return playersTurn ? players[currentPlayerIndex] : nullptr;
}

BlackjackContext BlackjackTable::buildContext(
		const BlackjackPlayer& player
) const {
	HandValue value = evaluate(player.getHand());
	BlackjackContext context;
	context.handValue = value.total;
	context.numCards = player.getHand().size();
	context.hasSoftAce = value.soft;
	context.dealerUpcard = cardValue(dealer.getHand().front());
	context.bet = bets.at(&player);
	context.playerChips = player.getChips();
	context.canDoubleDown =
			context.numCards == 2 && context.playerChips >= context.bet;
	return context;
}

void BlackjackTable::playDealerTurn() {
	int total = handTotal(dealer.getHand());
	observer->onDealerReveals(dealer, total);
	observer->onAnnouncement("Dealer reveals hole card...");
	while (total < DEALER_STAND_TOTAL) {
		dealer.dealTo(dealer);
		total = handTotal(dealer.getHand());
		observer->onDealerHits(dealer, total);
		observer->onAnnouncement("Dealer hits...");
	}
	announceDealerResult(total);
}

void BlackjackTable::announceDealerResult(int dealerTotal) {
	if (dealerTotal > BLACKJACK) {
		observer->onDealerBust();
		observer->onAnnouncement("Dealer busts!");
	} else {
		observer->onDealerStand(dealerTotal);
		observer->onAnnouncement("Dealer stands at " + std::to_string(dealerTotal));
	}
}

void BlackjackTable::settleBets() {
	state = BlackjackState::Resolving;
	for (BlackjackPlayer* player : players) {
		int bet = bets.at(player);
		Settlement result = settle(player->getHand(), dealer.getHand(), bet);
		player->receiveChips(bet + result.chipDelta);
		observer->onBetSettled(*player, result.outcome, result.chipDelta);
	}
	observer->onAnnouncement("Settling bets...");
	state = BlackjackState::HandComplete;
}

void BlackjackTable::collectCards() {
	returnCardsToDealer();
	bets.clear();
	currentPlayerIndex = 0;
	state = BlackjackState::WaitingForPlayers;
}

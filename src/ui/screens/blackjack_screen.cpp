#include "blackjack_screen.hpp"
#include "core/blackjack/blackjack_table.hpp"
#include "game_screen.hpp"
#include "ui/layout/table_layout.hpp"
#include "ui/session/session_observer.hpp"

using namespace ftxui;

namespace {

struct BlackjackSession : GameSession {
	using GameSession::GameSession;

	SeatView dealer;
	bool holeCardVisible = false;
	PendingDecision<BlackjackBetContext, int> bet;
	PendingDecision<BlackjackContext, BlackjackAction> decision;
};

class BlackjackHuman : public BlackjackPlayer {
public:
	BlackjackHuman(const GameConfig& config, BlackjackSession& session)
			: Player(config.playerName, config.startingChips), session(session) {}

	int decideBlackjackBet(const BlackjackBetContext& context) override {
		return session.awaitDecision(session.bet, context, context.minBet);
	}

	BlackjackAction decideBlackjackAction(
			const BlackjackContext& context
	) override {
		return session.awaitDecision(
				session.decision, context, BlackjackAction::stand()
		);
	}

private:
	BlackjackSession& session;
};

class BlackjackSessionObserver
		: public SessionObserver<BlackjackObserver, BlackjackSession> {
public:
	using SessionObserver::SessionObserver;

	void onBetPlaced(const Player& player, int amount) override {
		session.update([&] {
			session.seatOf(player.getName()).bet = amount;
			session.addLogEntry(player.getName() + " bets " + number(amount));
		});
	}

	void onDealingStarted() override { log("Dealing..."); }

	void onPlayerDealt(const Player& player, int total) override {
		showPlayer(player, "Value: " + number(total), "");
	}

	void onDealerDealt(const CardHolder& dealer) override {
		std::string upcard = dealer.getHand().front().toString();
		showDealer(dealer.getHand(), "Showing: " + upcard, "");
	}

	void onPlayerHit(const Player& player, int total) override {
		std::string value = number(total);
		showPlayer(
				player, "Hit (Value: " + value + ")",
				player.getName() + " hits -> " + value
		);
	}

	void onPlayerStand(const Player& player, int total) override {
		std::string value = number(total);
		showPlayer(
				player, "Stand (" + value + ")",
				player.getName() + " stands at " + value
		);
	}

	void onPlayerBust(const Player& player) override {
		showPlayer(player, "BUST!", player.getName() + " busts!");
	}

	void onPlayerDoubleDown(const Player& player, int newBet) override {
		std::string bet = number(newBet);
		session.update([&] { session.seatOf(player.getName()).bet = newBet; });
		showPlayer(
				player, "Double Down! (Bet: " + bet + ")",
				player.getName() + " doubles down (bet: " + bet + ")"
		);
	}

	void onDealerReveals(const CardHolder& dealer, int total) override {
		session.update([&] { session.holeCardVisible = true; });
		std::string value = number(total);
		showDealer(dealer.getHand(), "Value: " + value, "Dealer reveals: " + value);
	}

	void onDealerHits(const CardHolder& dealer, int total) override {
		std::string value = number(total);
		showDealer(dealer.getHand(), "Hits -> " + value, "Dealer hits -> " + value);
	}

	void onDealerBust() override {
		showDealer(session.dealer.cards, "BUST!", "Dealer busts!");
	}

	void onDealerStand(int total) override {
		std::string value = number(total);
		showDealer(
				session.dealer.cards, "Stands at " + value, "Dealer stands at " + value
		);
	}

	void onBetSettled(
			const Player& player, const std::string& outcome, int chipDelta
	) override {
		std::string sign = chipDelta >= 0 ? "+" : "";
		std::string change = " (" + sign + number(chipDelta) + ")";
		session.update([&] {
			session.seatOf(player.getName()).chips = player.getChips();
			session.addLogEntry(player.getName() + ": " + outcome + change);
		});
	}

private:
	static std::string number(int value) { return std::to_string(value); }

	void showPlayer(
			const Player& player, const std::string& status,
			const std::string& logEntry
	) {
		session.update([&] {
			SeatView& seat = session.seatOf(player.getName());
			seat.cards = player.getHand();
			seat.isThinking = false;
			seat.lastAction = status;
			if (!logEntry.empty()) session.addLogEntry(logEntry);
		});
	}

	void showDealer(
			const Cards& cards, const std::string& status, const std::string& logEntry
	) {
		session.update([&] {
			session.dealer.cards = cards;
			session.dealer.lastAction = status;
			if (!logEntry.empty()) session.addLogEntry(logEntry);
		});
	}
};

class BlackjackScreen : public GameScreen {
public:
	explicit BlackjackScreen(const GameConfig& config)
			: GameScreen(config), state(screen),
				blackjackTable(config.stakes.low, config.stakes.high),
				human(config, state), observer(state) {
		seatPlayers(blackjackTable, human);
		blackjackTable.setObserver(&observer);
	}

protected:
	GameSession& session() override { return state; }

	TableBase& table() override { return blackjackTable; }

	std::string gameTitle() const override { return "Blackjack"; }

	Elements headerDetails() override {
		std::string range = betRange(config.stakes.low, config.stakes.high);
		return {text(" Bets: " + range + " ")};
	}

	Element renderTable() override {
		return TableLayout::renderBlackjackTable(
				state.seats, state.dealer, state.holeCardVisible
		);
	}

	Element renderDecisionBar() override {
		if (state.bet.waiting) return renderBetBar();
		if (state.decision.waiting) return renderPlayChoices();
		return nullptr;
	}

	bool handleDecisionEvent(const Event& event) override {
		if (state.bet.waiting) return handleBetEvent(event);
		if (state.decision.waiting) return handleActionEvent(event);
		return false;
	}

	void resetTableView() override {
		state.holeCardVisible = false;
		state.dealer = SeatView();
		state.dealer.name = "Dealer";
	}

	void decorateSeat(SeatView& seat, int) override { seat.showCards = true; }

private:
	static std::string betRange(int min, int max) {
		return std::to_string(min) + "-" + std::to_string(max);
	}

	Element renderBetBar() {
		const BlackjackBetContext& context = state.bet.context;
		std::string range = betRange(context.minBet, context.maxBet);
		Element bar = hbox({
				text(" Place your bet (" + range + "): ") | bold,
				renderAmountInput(),
				text(" (Enter to confirm) ") | dim,
		});
		return bar | center;
	}

	Element renderPlayChoices() {
		Elements actions = {
				text(" [H]it ") | bold | color(Color::Green),
				text("  "),
				text(" [S]tand ") | bold | color(Color::Cyan),
				text("  "),
		};
		if (state.decision.context.canDoubleDown) {
			actions.push_back(text(" [D]ouble Down ") | bold | color(Color::Magenta));
		}
		return hbox(actions) | center;
	}

	bool handleBetEvent(const Event& event) {
		if (event != Event::Return) return false;
		std::optional<int> amount = takeEnteredAmount();
		if (!amount) return false;
		const BlackjackBetContext& context = state.bet.context;
		int bet = std::clamp(*amount, context.minBet, context.maxBet);
		state.respond(state.bet, bet);
		return true;
	}

	bool handleActionEvent(const Event& event) {
		std::optional<BlackjackAction> action = actionForEvent(event);
		if (action) state.respond(state.decision, *action);
		return action.has_value();
	}

	std::optional<BlackjackAction> actionForEvent(const Event& event) {
		if (isKey(event, 'h')) return BlackjackAction::hit();
		if (isKey(event, 's')) return BlackjackAction::stand();
		bool canDouble = state.decision.context.canDoubleDown;
		if (isKey(event, 'd') && canDouble) return BlackjackAction::doubleDown();
		return std::nullopt;
	}

	BlackjackSession state;
	BlackjackTable blackjackTable;
	BlackjackHuman human;
	BlackjackSessionObserver observer;
};

} // namespace

bool runBlackjackScreen(const GameConfig& config) {
	return BlackjackScreen(config).run();
}

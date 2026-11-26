#include "poker_screen.hpp"
#include "core/common/text.hpp"
#include "core/poker/poker_table.hpp"
#include "game_screen.hpp"
#include "ui/layout/table_layout.hpp"
#include "ui/session/session_observer.hpp"

using namespace ftxui;

namespace {

struct PokerSession : GameSession {
	using GameSession::GameSession;

	Cards communityCards;
	int pot = 0;
	std::string streetName;
	PendingDecision<BettingContext, PokerAction> decision;
};

class PokerHuman : public PokerPlayer {
public:
	PokerHuman(const GameConfig& config, PokerSession& session)
			: Player(config.playerName, config.startingChips), session(session) {}

	PokerAction decidePokerAction(const BettingContext& context) override {
		return session.awaitDecision(
				session.decision, context, PokerAction::fold()
		);
	}

private:
	PokerSession& session;
};

void showActionOnSeat(SeatView& seat, const PokerAction& action) {
	std::string amount = std::to_string(action.amount);
	switch (action.type) {
		case ActionType::Fold:
			seat.isFolded = true;
			seat.lastAction = "Fold";
			break;
		case ActionType::Check:
			seat.lastAction = "Check";
			break;
		case ActionType::Call:
			seat.lastAction = "Call " + amount;
			seat.bet += action.amount;
			break;
		case ActionType::Raise:
			seat.lastAction = "Raise to " + amount;
			seat.bet = action.amount;
			break;
		case ActionType::AllIn:
			seat.isAllIn = true;
			seat.lastAction = "ALL-IN " + amount;
			seat.bet = action.amount;
			break;
	}
}

class PokerSessionObserver
		: public SessionObserver<PokerObserver, PokerSession> {
public:
	using SessionObserver::SessionObserver;

	void onHandStarted(const Player& button, int, int) override {
		session.update([&] {
			session.streetName = "Dealing...";
			session.addLogEntry(
					"Hand #" + std::to_string(session.handNumber)
					+ " - Dealer: " + button.getName()
			);
		});
	}

	void onHoleCardsDealt(const Player& player) override {
		session.update([&] {
			session.seatOf(player.getName()).cards = player.getHand();
		});
	}

	void onBlindPosted(
			const Player& player, int amount, const std::string& blindName, int pot
	) override {
		std::string label = blindName + " (" + std::to_string(amount) + ")";
		session.update([&] {
			SeatView& seat = session.seatOf(player.getName());
			seat.bet = amount;
			seat.chips = player.getChips();
			seat.lastAction = label;
			session.pot = pot;
			session.addLogEntry(player.getName() + " posts " + label);
		});
	}

	void onStreetStarted(const std::string& street, const Cards& cards) override {
		session.update([&] {
			session.streetName = street;
			session.communityCards = cards;
			for (SeatView& seat : session.seats) {
				seat.bet = 0;
				seat.lastAction.clear();
			}
			session.addLogEntry("--- " + street + " ---");
		});
	}

	void onPlayerAction(
			const Player& player, const PokerAction& action, int pot
	) override {
		std::string entry = player.getName() + " " + action.getActionName();
		if (action.amount > 0) entry += " " + std::to_string(action.amount);
		session.update([&] {
			SeatView& seat = session.seatOf(player.getName());
			seat.isThinking = false;
			seat.chips = player.getChips();
			showActionOnSeat(seat, action);
			session.pot = pot;
			session.addLogEntry(entry);
		});
	}

	void onShowdownBegin(const Cards&) override {
		session.update([&] {
			session.streetName = "Showdown";
			for (SeatView& seat : session.seats) {
				seat.showCards = !seat.isFolded;
			}
			session.addLogEntry("--- Showdown ---");
		});
	}

	void onShowdownReveal(
			const Player& player, const std::string& hand
	) override {
		log(player.getName() + ": " + hand);
	}

	void onPotWon(
			const Player& winner, int amount, const std::string& reason
	) override {
		std::string share = std::to_string(amount);
		logPayout(winner.getName() + " wins " + share + " (" + reason + ")");
	}

	void onSplitPot(
			const std::vector<const Player*>& winners, int share,
			const std::string& hand
	) override {
		std::vector<std::string> names;
		for (const Player* winner : winners) {
			names.push_back(winner->getName());
		}
		std::string each = std::to_string(share) + " each, " + hand;
		logPayout("Split pot: " + join(names, ", ") + " (" + each + ")");
	}

private:
	void logPayout(const std::string& entry) {
		session.update([&] {
			session.pot = 0;
			session.addLogEntry(entry);
		});
	}
};

class PokerScreen : public GameScreen {
public:
	explicit PokerScreen(const GameConfig& config)
			: GameScreen(config), state(screen),
				pokerTable(config.stakes.low, config.stakes.high), human(config, state),
				observer(state) {
		seatPlayers(pokerTable, human);
		pokerTable.setObserver(&observer);
	}

protected:
	GameSession& session() override { return state; }

	TableBase& table() override { return pokerTable; }

	std::string gameTitle() const override { return "Poker"; }

	Elements headerDetails() override {
		std::string blinds = std::to_string(config.stakes.low) + "/"
				+ std::to_string(config.stakes.high);
		return {
				text(" Blinds " + blinds + " "),
				text(" Pot: " + std::to_string(state.pot) + " ") | bold
						| color(Color::Yellow),
				text(" " + state.streetName + " ") | color(Color::Cyan),
		};
	}

	Element renderTable() override {
		return TableLayout::renderPokerTable(
				state.seats, state.communityCards, state.pot, state.streetName
		);
	}

	Element renderDecisionBar() override {
		if (!state.decision.waiting) return nullptr;
		int costToCall = state.decision.context.costToCall;
		std::string callLabel =
				costToCall == 0 ? "[C]heck" : "[C]all " + std::to_string(costToCall);
		Element bar = hbox({
				text(" [F]old ") | bold | color(Color::Red),
				text("  "),
				text(" " + callLabel + " ") | bold | color(Color::Green),
				text("  "),
				text(" [A]ll-In ") | bold | color(Color::Magenta),
				text("  "),
				text(" Raise: "),
				renderAmountInput(),
				text(" (Enter to raise) ") | dim,
		});
		return bar | center;
	}

	bool handleDecisionEvent(const Event& event) override {
		if (!state.decision.waiting) return false;
		std::optional<PokerAction> action = actionForEvent(event);
		if (action) state.respond(state.decision, *action);
		return action.has_value();
	}

	void resetTableView() override {
		state.communityCards.clear();
		state.pot = 0;
	}

	void decorateSeat(SeatView& seat, int seatIndex) override {
		seat.isDealer = seatIndex == pokerTable.getDealerButton();
	}

private:
	std::optional<PokerAction> actionForEvent(const Event& event) {
		if (isKey(event, 'f')) return PokerAction::fold();
		if (isKey(event, 'a')) return PokerAction::allIn();
		if (isKey(event, 'c')) {
			bool free = state.decision.context.costToCall == 0;
			return free ? PokerAction::check() : PokerAction::call();
		}
		if (event != Event::Return) return std::nullopt;
		std::optional<int> amount = takeEnteredAmount();
		if (!amount || *amount <= 0) return std::nullopt;
		return PokerAction::raise(*amount);
	}

	PokerSession state;
	PokerTable pokerTable;
	PokerHuman human;
	PokerSessionObserver observer;
};

} // namespace

bool runPokerScreen(const GameConfig& config) {
	return PokerScreen(config).run();
}

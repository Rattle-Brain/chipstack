#pragma once

#include "ai/bot_player.hpp"
#include "core/table/table.hpp"
#include "ui/game_config.hpp"
#include "ui/session/game_session.hpp"
#include <ftxui/component/component.hpp>
#include <memory>

class GameScreen {
public:
	explicit GameScreen(const GameConfig& config);
	virtual ~GameScreen() = default;

	bool run();

protected:
	virtual GameSession& session() = 0;
	virtual TableBase& table() = 0;
	virtual std::string gameTitle() const = 0;
	virtual ftxui::Elements headerDetails() = 0;
	virtual ftxui::Element renderTable() = 0;
	virtual ftxui::Element renderDecisionBar() = 0;
	virtual bool handleDecisionEvent(const ftxui::Event& event) = 0;
	virtual void resetTableView() = 0;

	virtual void decorateSeat(SeatView& seat, int seatIndex) {}

	template <class TableT, class HumanT>
	void seatPlayers(TableT& gameTable, HumanT& human) {
		humanPlayer = &human;
		gameTable.addPlayer(&human);
		for (const std::unique_ptr<BotPlayer>& bot : bots) {
			gameTable.addPlayer(bot.get());
		}
	}

	static bool isKey(const ftxui::Event& event, char key);
	std::optional<int> takeEnteredAmount();
	ftxui::Element renderAmountInput();

	const GameConfig& config;
	ftxui::ScreenInteractive screen;

private:
	void playHands();
	bool handLimitReached(int handNumber) const;
	void playHand(int handNumber);
	void prepareHandView(int handNumber);
	void finishHandView();
	std::optional<std::string> finalResult();
	void endGame(const std::string& message);
	void stopGame();

	bool handleEvent(const ftxui::Event& event);
	bool handlePauseEvent(const ftxui::Event& event);
	ftxui::Element render();
	ftxui::Element renderHeader();
	ftxui::Element renderAnnouncement();
	ftxui::Element renderActionBar();
	ftxui::Element renderLog();

	std::vector<std::unique_ptr<BotPlayer>> bots;
	const Player* humanPlayer = nullptr;
	std::string amountText;
	ftxui::Component amountInput;
};

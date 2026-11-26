#include "game_catalog.hpp"
#include "screens/blackjack_screen.hpp"
#include "screens/poker_screen.hpp"

namespace {

constexpr Difficulty NOVICE = Difficulty::Novice;
constexpr Difficulty GOOD = Difficulty::Good;
constexpr Difficulty GREAT = Difficulty::Great;
constexpr Difficulty PRO = Difficulty::Professional;

GameDefinition pokerDefinition() {
	return {
			"Poker",
			"Poker (NL Hold'em)",
			{
					{
							"Casual Home Game",
							"5/10 blinds, 3 Novice bots",
							{5, 10},
							{{"Timmy", NOVICE}, {"Bobby", NOVICE}, {"Sally", NOVICE}},
					},
					{
							"Friday Night",
							"10/20 blinds, Good+Great+Pro",
							{10, 20},
							{{"Rocky", GOOD}, {"Sharp", GREAT}, {"Ace", PRO}},
					},
					{
							"Shark Tank",
							"25/50 blinds, Great+2 Pro",
							{25, 50},
							{{"Sharp", GREAT}, {"Ace", PRO}, {"Viper", PRO}},
					},
					{
							"The Grind",
							"10/20 blinds, 4 mixed bots",
							{10, 20},
							{
									{"Timmy", NOVICE},
									{"Rocky", GOOD},
									{"Sharp", GREAT},
									{"Ace", PRO},
							},
					},
			},
			{"Small Blind", 5, 100, 5},
			{"Big Blind", 10, 200, 10},
			{10, 20},
			3,
			runPokerScreen,
	};
}

GameDefinition blackjackDefinition() {
	return {
			"Blackjack",
			"Blackjack",
			{
					{
							"Low Stakes",
							"10-100 bets, 2 Novice bots",
							{10, 100},
							{{"Timmy", NOVICE}, {"Bobby", NOVICE}},
					},
					{
							"Standard Table",
							"25-500 bets, Good+Great+Pro",
							{25, 500},
							{{"Rocky", GOOD}, {"Sharp", GREAT}, {"Ace", PRO}},
					},
					{
							"High Roller",
							"100-2000 bets, 2 Pro bots",
							{100, 2000},
							{{"Ace", PRO}, {"Viper", PRO}},
					},
			},
			{"Min Bet", 5, 500, 5},
			{"Max Bet", 100, 5000, 100},
			{10, 500},
			2,
			runBlackjackScreen,
	};
}

} // namespace

const std::vector<GameDefinition>& gameCatalog() {
	static const std::vector<GameDefinition> catalog = {
			pokerDefinition(), blackjackDefinition()
	};
	return catalog;
}

void applyScenario(const Scenario& scenario, GameConfig& config) {
	config.stakes = scenario.stakes;
	config.handLimit = 0;
	config.bots.clear();
	for (const auto& [name, difficulty] : scenario.bots) {
		config.bots.push_back({name, presetPersonality(difficulty)});
	}
}

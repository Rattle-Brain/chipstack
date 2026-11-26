#include "ai/bot_player.hpp"
#include "core/poker/betting_context.hpp"
#include <gtest/gtest.h>

static BettingContext makeBasicContext() {
	BettingContext ctx;
	ctx.roundType = BettingRoundType::PreFlop;
	ctx.pot = 30;
	ctx.currentBet = 20;
	ctx.playerCurrentBet = 0;
	ctx.costToCall = 20;
	ctx.playerChips = 1000;
	ctx.numActivePlayers = 3;
	ctx.numPlayersToAct = 2;
	ctx.minRaise = 20;
	ctx.bigBlind = 20;
	ctx.dealerButton = 0;
	ctx.playerSeatIndex = 2;
	ctx.totalPlayers = 3;
	ctx.position = Position::Late;
	return ctx;
}

TEST(BotPlayerTest, NoviceReturnsValidAction) {
	BotPlayer bot("Novice", 1000, {0.25f, 0.15f, Difficulty::Novice});
	bot.receiveCard(Card(14, Spades));
	bot.receiveCard(Card(13, Hearts));

	auto ctx = makeBasicContext();
	PokerAction action = bot.decidePokerAction(ctx);

	EXPECT_TRUE(
			action.type == ActionType::Fold || action.type == ActionType::Check
			|| action.type == ActionType::Call || action.type == ActionType::Raise
			|| action.type == ActionType::AllIn
	);
}

TEST(BotPlayerTest, GoodReturnsValidAction) {
	BotPlayer bot("Good", 1000, {0.60f, 0.50f, Difficulty::Good});
	bot.receiveCard(Card(14, Spades));
	bot.receiveCard(Card(13, Hearts));

	auto ctx = makeBasicContext();
	PokerAction action = bot.decidePokerAction(ctx);

	EXPECT_TRUE(
			action.type == ActionType::Fold || action.type == ActionType::Check
			|| action.type == ActionType::Call || action.type == ActionType::Raise
			|| action.type == ActionType::AllIn
	);
}

TEST(BotPlayerTest, GreatReturnsValidAction) {
	BotPlayer bot("Great", 1000, {0.50f, 0.65f, Difficulty::Great});
	bot.receiveCard(Card(14, Spades));
	bot.receiveCard(Card(13, Hearts));

	auto ctx = makeBasicContext();
	PokerAction action = bot.decidePokerAction(ctx);

	EXPECT_TRUE(
			action.type == ActionType::Fold || action.type == ActionType::Check
			|| action.type == ActionType::Call || action.type == ActionType::Raise
			|| action.type == ActionType::AllIn
	);
}

TEST(BotPlayerTest, ProfessionalReturnsValidAction) {
	BotPlayer bot("Pro", 1000, {0.75f, 0.55f, Difficulty::Professional});
	bot.receiveCard(Card(14, Spades));
	bot.receiveCard(Card(13, Hearts));

	auto ctx = makeBasicContext();
	PokerAction action = bot.decidePokerAction(ctx);

	EXPECT_TRUE(
			action.type == ActionType::Fold || action.type == ActionType::Check
			|| action.type == ActionType::Call || action.type == ActionType::Raise
			|| action.type == ActionType::AllIn
	);
}

TEST(BotPlayerTest, ProfessionalRarelyFoldsMonsters) {
	int foldCount = 0;
	int trials = 100;

	for (int i = 0; i < trials; i++) {
		BotPlayer bot("Pro", 1000, {0.75f, 0.55f, Difficulty::Professional});
		bot.receiveCard(Card(14, Spades));
		bot.receiveCard(Card(14, Hearts));

		auto ctx = makeBasicContext();
		PokerAction action = bot.decidePokerAction(ctx);
		if (action.type == ActionType::Fold) foldCount++;
	}

	EXPECT_LT(foldCount, 10) << "Professional should rarely fold pocket aces";
}

TEST(BotPlayerTest, NoviceHasVariance) {
	std::set<int> actionTypes;
	for (int i = 0; i < 200; i++) {
		BotPlayer bot("Novice", 1000, {0.25f, 0.15f, Difficulty::Novice});
		bot.receiveCard(Card(10, Spades));
		bot.receiveCard(Card(9, Hearts));

		auto ctx = makeBasicContext();
		PokerAction action = bot.decidePokerAction(ctx);
		actionTypes.insert(static_cast<int>(action.type));
	}

	EXPECT_GT(actionTypes.size(), 1u)
			<< "Novice should show variance in decisions";
}

TEST(BotPlayerTest, BlackjackProfessionalStandsHard17) {
	int standCount = 0;
	int trials = 50;

	for (int i = 0; i < trials; i++) {
		BotPlayer bot("Pro", 1000, {0.75f, 0.55f, Difficulty::Professional});

		BlackjackContext ctx;
		ctx.handValue = 17;
		ctx.numCards = 2;
		ctx.hasSoftAce = false;
		ctx.dealerUpcard = 10;
		ctx.bet = 50;
		ctx.playerChips = 950;
		ctx.canDoubleDown = false;

		BlackjackAction action = bot.decideBlackjackAction(ctx);
		if (action.type == BlackjackActionType::Stand) standCount++;
	}

	EXPECT_EQ(standCount, trials)
			<< "Professional should always stand on hard 17";
}

TEST(BotPlayerTest, BlackjackProfessionalHitsBelow12) {
	int hitCount = 0;
	int trials = 50;

	for (int i = 0; i < trials; i++) {
		BotPlayer bot("Pro", 1000, {0.75f, 0.55f, Difficulty::Professional});

		BlackjackContext ctx;
		ctx.handValue = 8;
		ctx.numCards = 2;
		ctx.hasSoftAce = false;
		ctx.dealerUpcard = 10;
		ctx.bet = 50;
		ctx.playerChips = 950;
		ctx.canDoubleDown = true;

		BlackjackAction action = bot.decideBlackjackAction(ctx);
		if (action.type == BlackjackActionType::Hit
				|| action.type == BlackjackActionType::DoubleDown)
			hitCount++;
	}

	EXPECT_EQ(hitCount, trials) << "Professional should always hit/double on 8";
}

TEST(BotPlayerTest, BetWithinBounds) {
	for (int i = 0; i < 100; i++) {
		BotPlayer bot("Test", 1000, {0.60f, 0.50f, Difficulty::Good});

		BlackjackBetContext ctx;
		ctx.playerChips = 1000;
		ctx.minBet = 25;
		ctx.maxBet = 500;

		int bet = bot.decideBlackjackBet(ctx);
		EXPECT_GE(bet, ctx.minBet);
		EXPECT_LE(bet, ctx.maxBet);
		EXPECT_LE(bet, ctx.playerChips);
	}
}

TEST(BotPlayerTest, DifficultyGetters) {
	BotPlayer bot("Test", 1000, {0.60f, 0.50f, Difficulty::Good});
	EXPECT_EQ(bot.getPersonality().difficulty, Difficulty::Good);
	EXPECT_STREQ(difficultyName(bot.getPersonality().difficulty), "Good");
	EXPECT_FLOAT_EQ(bot.getPersonality().riskTolerance, 0.60f);
}

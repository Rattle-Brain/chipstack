#pragma once

enum class BlackjackActionType { Hit, Stand, DoubleDown };

struct BlackjackAction {
	BlackjackActionType type;

	static BlackjackAction hit() { return {BlackjackActionType::Hit}; }
	static BlackjackAction stand() { return {BlackjackActionType::Stand}; }

	static BlackjackAction doubleDown() {
		return {BlackjackActionType::DoubleDown};
	}

	const char* getActionName() const {
		static const char* names[] = {"Hit", "Stand", "Double Down"};
		return names[static_cast<int>(type)];
	}
};

struct BlackjackContext {
	int handValue;
	int numCards;
	bool hasSoftAce;
	int dealerUpcard;
	int bet;
	int playerChips;
	bool canDoubleDown;
};

struct BlackjackBetContext {
	int playerChips;
	int minBet;
	int maxBet;
};

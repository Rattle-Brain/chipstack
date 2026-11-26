#pragma once

enum class ActionType { Fold, Check, Call, Raise, AllIn };

struct PokerAction {
	ActionType type;
	int amount = 0;

	static PokerAction fold() { return {ActionType::Fold}; }
	static PokerAction check() { return {ActionType::Check}; }
	static PokerAction call() { return {ActionType::Call}; }
	static PokerAction raise(int amount) { return {ActionType::Raise, amount}; }
	static PokerAction allIn() { return {ActionType::AllIn}; }

	const char* getActionName() const {
		static const char* names[] = {"Fold", "Check", "Call", "Raise", "All-In"};
		return names[static_cast<int>(type)];
	}
};

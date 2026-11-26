#pragma once

enum class Difficulty { Novice, Good, Great, Professional };

struct BotPersonality {
	float riskTolerance;
	float aggressiveness;
	Difficulty difficulty;
};

const char* difficultyName(Difficulty difficulty);
BotPersonality presetPersonality(Difficulty difficulty);
BotPersonality normalizePersonality(BotPersonality personality);

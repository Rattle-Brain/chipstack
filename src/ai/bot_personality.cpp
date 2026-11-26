#include "bot_personality.hpp"
#include <algorithm>

const char* difficultyName(Difficulty difficulty) {
	static const char* names[] = {"Novice", "Good", "Great", "Professional"};
	return names[static_cast<int>(difficulty)];
}

BotPersonality presetPersonality(Difficulty difficulty) {
	static const float presets[][2] = {
			{0.25f, 0.15f}, {0.60f, 0.50f}, {0.50f, 0.65f}, {0.75f, 0.55f}
	};
	const float* preset = presets[static_cast<int>(difficulty)];
	return {preset[0], preset[1], difficulty};
}

BotPersonality normalizePersonality(BotPersonality personality) {
	float risk = std::clamp(personality.riskTolerance, 0.0f, 1.0f);
	float maxAggression = std::min(1.0f, risk + 0.15f);
	float aggression =
			std::clamp(personality.aggressiveness, 0.0f, maxAggression);
	return {risk, aggression, personality.difficulty};
}

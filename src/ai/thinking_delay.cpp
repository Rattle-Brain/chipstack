#include "thinking_delay.hpp"

std::chrono::milliseconds ThinkingDelay::generate() {
	using std::chrono::milliseconds;
	if (!enabled) return milliseconds(0);
	float roll = random.uniform(0.0f, 1.0f);
	if (roll < 0.10f) return milliseconds(0);
	if (roll < 0.95f) return milliseconds(random.uniformInt(500, 3000));
	return milliseconds(random.uniformInt(3000, 6000));
}

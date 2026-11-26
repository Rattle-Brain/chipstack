#pragma once

#include "random_source.hpp"
#include <chrono>

class ThinkingDelay {
public:
	explicit ThinkingDelay(bool enabled = true) : enabled(enabled) {}

	std::chrono::milliseconds generate();

	void setEnabled(bool isEnabled) { enabled = isEnabled; }
	bool isEnabled() const { return enabled; }

private:
	bool enabled;
	RandomSource random;
};

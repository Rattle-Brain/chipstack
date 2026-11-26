#pragma once

#include <random>

class RandomSource {
public:
	RandomSource() : generator(std::random_device{}()) {}

	float uniform(float min, float max) {
		return std::uniform_real_distribution<float>(min, max)(generator);
	}

	int uniformInt(int min, int max) {
		return std::uniform_int_distribution<int>(min, max)(generator);
	}

	bool chance(float probability) { return uniform(0.0f, 1.0f) < probability; }

private:
	std::mt19937 generator;
};

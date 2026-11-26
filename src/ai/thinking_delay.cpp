#include "thinking_delay.hpp"

ThinkingDelay::ThinkingDelay(bool enabled)
    : enabled_(enabled), rng_(std::random_device{}()) {}

std::chrono::milliseconds ThinkingDelay::generate() {
    if (!enabled_) return std::chrono::milliseconds(0);

    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    float roll = dist(rng_);

    if (roll < 0.10f) {
        return std::chrono::milliseconds(0);
    } else if (roll < 0.95f) {
        std::uniform_int_distribution<int> delay(500, 3000);
        return std::chrono::milliseconds(delay(rng_));
    } else {
        std::uniform_int_distribution<int> delay(3000, 6000);
        return std::chrono::milliseconds(delay(rng_));
    }
}

#ifndef THINKING_DELAY_HPP
#define THINKING_DELAY_HPP

#include <chrono>
#include <random>

class ThinkingDelay {
  public:
    ThinkingDelay(bool enabled = true);

    std::chrono::milliseconds generate();

    void setEnabled(bool enabled) { enabled_ = enabled; }
    bool isEnabled() const { return enabled_; }

  private:
    bool enabled_;
    std::mt19937 rng_;
};

#endif

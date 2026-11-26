#include <gtest/gtest.h>
#include "ai/thinking_delay.hpp"

TEST(ThinkingDelayTest, DisabledReturnsZero) {
    ThinkingDelay delay(false);
    for (int i = 0; i < 100; i++) {
        EXPECT_EQ(delay.generate().count(), 0);
    }
}

TEST(ThinkingDelayTest, EnabledReturnsNonNegative) {
    ThinkingDelay delay(true);
    for (int i = 0; i < 100; i++) {
        EXPECT_GE(delay.generate().count(), 0);
    }
}

TEST(ThinkingDelayTest, DistributionRoughlyCorrect) {
    ThinkingDelay delay(true);
    int immediate = 0;
    int normal = 0;
    int slow = 0;
    int total = 2000;

    for (int i = 0; i < total; i++) {
        auto ms = delay.generate().count();
        if (ms == 0) {
            immediate++;
        } else if (ms <= 3000) {
            normal++;
        } else {
            slow++;
        }
    }

    double immPct = (double)immediate / total;
    double normPct = (double)normal / total;
    double slowPct = (double)slow / total;

    EXPECT_NEAR(immPct, 0.10, 0.05) << "~10% should be immediate";
    EXPECT_NEAR(normPct, 0.85, 0.08) << "~85% should be 0.5-3s";
    EXPECT_NEAR(slowPct, 0.05, 0.04) << "~5% should be 3-6s";
}

TEST(ThinkingDelayTest, MaxDelayNotExceeded) {
    ThinkingDelay delay(true);
    for (int i = 0; i < 500; i++) {
        EXPECT_LE(delay.generate().count(), 6000);
    }
}

TEST(ThinkingDelayTest, SetEnabled) {
    ThinkingDelay delay(true);
    EXPECT_TRUE(delay.isEnabled());
    delay.setEnabled(false);
    EXPECT_FALSE(delay.isEnabled());
    EXPECT_EQ(delay.generate().count(), 0);
}

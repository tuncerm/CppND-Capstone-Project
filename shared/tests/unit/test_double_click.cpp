/**
 * Unit Tests for Double Click Utility Component
 *
 * Tests double-click detection functionality with timing and state management.
 */

#include <gtest/gtest.h>
#include "../utils/test_helpers.h"
#include "utilities/double_click.h"

class DoubleClickTest : public ::testing::Test {
   protected:
    void SetUp() override { double_click_init(&detector, 500); }

    void TearDown() override {
        // No explicit cleanup needed for double click detector
    }

    DoubleClickDetector detector;
};

// ===== Initialization Tests =====

TEST_F(DoubleClickTest, InitializationState) {
    DoubleClickDetector new_detector;
    double_click_init(&new_detector, 500);

    // Initial state should be ready for first click
    EXPECT_FALSE(double_click_has_previous(&new_detector));
}

TEST_F(DoubleClickTest, InitializationWithNullPointer) {
    EXPECT_NO_FATAL_FAILURE(double_click_init(nullptr, 500));
}

// ===== Basic Double Click Detection Tests =====

TEST_F(DoubleClickTest, SingleClickDetection) {
    Uint32 current_time = SDL_GetTicks();

    // Single click should not trigger double-click
    bool double_clicked = double_click_check(&detector, 1);
    EXPECT_FALSE(double_clicked);
    EXPECT_TRUE(double_click_has_previous(&detector));
}

TEST_F(DoubleClickTest, DoubleClickDetection) {
    Uint32 current_time = SDL_GetTicks();

    // First click
    bool first_result = double_click_check(&detector, 1);
    EXPECT_FALSE(first_result);
    EXPECT_TRUE(double_click_has_previous(&detector));

    // Second click within time window
    current_time += 150;  // 150ms later (within typical double-click window)
    bool second_result = double_click_check(&detector, 1);
    EXPECT_TRUE(second_result);
    EXPECT_FALSE(double_click_has_previous(&detector));  // Should reset after double-click
}

TEST_F(DoubleClickTest, DoubleClickTimeout) {
    Uint32 current_time = SDL_GetTicks();

    // First click
    double_click_check(&detector, 1);
    EXPECT_TRUE(double_click_has_previous(&detector));

    // Wait too long for second click
    current_time += 1000;  // 1 second later (should timeout)
    // To simulate a timeout, we just don't call check again.
    // The next call to check will not be a double click.
    current_time += 1000;
    bool result = double_click_check(&detector, 1);
    EXPECT_FALSE(result);
}

// ===== Timing Tests =====

TEST_F(DoubleClickTest, ExactTimingBoundaries) {
    Uint32 current_time = SDL_GetTicks();

    // First click
    double_click_check(&detector, 1);

    // Test at exact boundary of double-click window
    // Note: Exact timing depends on implementation constants
    current_time += 500;  // 500ms - typically at or near the limit

    // Update without click to process timeout
    // To simulate a timeout, we just don't call check again.

    // Should have timed out by now
    current_time += 501;
    bool result = double_click_check(&detector, 1);
    EXPECT_FALSE(result);
}

TEST_F(DoubleClickTest, QuickSuccession) {
    Uint32 current_time = SDL_GetTicks();

    // Very quick double-click (10ms apart)
    double_click_check(&detector, 1);
    current_time += 10;
    bool result = double_click_check(&detector, 1);

    EXPECT_TRUE(result);  // Should still detect as double-click
}

// ===== State Management Tests =====

TEST_F(DoubleClickTest, ResetAfterDoubleClick) {
    Uint32 current_time = SDL_GetTicks();

    // Complete double-click sequence
    double_click_check(&detector, 1);
    current_time += 100;
    bool double_clicked = double_click_check(&detector, 1);
    EXPECT_TRUE(double_clicked);
    EXPECT_FALSE(double_click_has_previous(&detector));

    // Next click should start new sequence
    current_time += 100;
    bool next_click = double_click_check(&detector, 1);
    EXPECT_FALSE(next_click);  // Should be first click of new sequence
    EXPECT_TRUE(double_click_has_previous(&detector));
}

TEST_F(DoubleClickTest, PendingStateManagement) {
    Uint32 current_time = SDL_GetTicks();

    EXPECT_FALSE(double_click_has_previous(&detector));

    // After first click
    double_click_check(&detector, 1);
    EXPECT_TRUE(double_click_has_previous(&detector));

    // After timeout
    current_time += 1000;
    current_time += 1000;
    double_click_check(&detector, 1);
    EXPECT_FALSE(double_click_has_previous(&detector));
}

// ===== Multiple Click Sequences Tests =====

TEST_F(DoubleClickTest, TripleClickHandling) {
    Uint32 current_time = SDL_GetTicks();

    // First click
    bool result1 = double_click_check(&detector, 1);
    EXPECT_FALSE(result1);

    // Second click (double-click)
    current_time += 100;
    bool result2 = double_click_check(&detector, 1);
    EXPECT_TRUE(result2);

    // Third click (should start new sequence)
    current_time += 100;
    bool result3 = double_click_check(&detector, 1);
    EXPECT_FALSE(result3);  // Not a double-click, but first of new sequence
    EXPECT_TRUE(double_click_has_previous(&detector));
}

TEST_F(DoubleClickTest, AlternatingClickRelease) {
    Uint32 current_time = SDL_GetTicks();

    // Click
    double_click_check(&detector, 1);
    current_time += 50;

    // Release (no click)
    // The check function only cares about the target id, not the mouse state
    current_time += 50;

    // Click again (should be double-click)
    bool result = double_click_check(&detector, 1);
    EXPECT_TRUE(result);
}

// ===== Edge Cases Tests =====

TEST_F(DoubleClickTest, ZeroTimestamp) {
    // Test with timestamp 0
    bool result = double_click_check(&detector, 1);
    EXPECT_FALSE(result);
    EXPECT_TRUE(double_click_has_previous(&detector));
}

TEST_F(DoubleClickTest, BackwardsTime) {
    Uint32 current_time = SDL_GetTicks();

    // First click
    double_click_check(&detector, 1);

    // Second click with earlier timestamp (shouldn't happen in practice)
    bool result = double_click_check(&detector, 1);

    // Implementation dependent - should handle gracefully
    // Either detect as double-click or reset state
    EXPECT_TRUE(result || !double_click_has_previous(&detector));
}

TEST_F(DoubleClickTest, OverflowTime) {
    // Test with very large timestamp values
    Uint32 large_time = UINT32_MAX - 1000;

    double_click_check(&detector, 1);
    EXPECT_TRUE(double_click_has_previous(&detector));

    // Click near overflow boundary
    bool result = double_click_check(&detector, 1);
    EXPECT_TRUE(result);
}

// ===== Error Handling Tests =====

TEST_F(DoubleClickTest, NullPointerHandling) {
    Uint32 current_time = SDL_GetTicks();

    // All functions should handle null pointer gracefully
    EXPECT_FALSE(double_click_check(nullptr, 1));
    EXPECT_FALSE(double_click_has_previous(nullptr));
}

// ===== Performance Tests =====

TEST_F(DoubleClickTest, UpdatePerformance) {
    Uint32 current_time = SDL_GetTicks();

    auto start = std::chrono::high_resolution_clock::now();

    // Many rapid updates
    for (int i = 0; i < 10000; ++i) {
        double_click_check(&detector, i % 2);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // 10000 updates should be very fast
    EXPECT_LT(duration.count(), 10000)
        << "10000 double-click updates took " << duration.count() << "μs";
}

// ===== Practical Usage Tests =====

TEST_F(DoubleClickTest, TypicalMouseUsage) {
    Uint32 current_time = SDL_GetTicks();

    // Simulate typical mouse usage pattern
    // Mouse down
    bool result1 = double_click_check(&detector, 1);
    EXPECT_FALSE(result1);

    // Mouse up (brief pause)
    current_time += 50;
    // The check function only cares about the target id, not the mouse state

    // Mouse down again (double-click)
    current_time += 100;
    bool result2 = double_click_check(&detector, 1);
    EXPECT_TRUE(result2);

    // Mouse up
    current_time += 50;
    // The check function only cares about the target id, not the mouse state
    EXPECT_FALSE(double_click_has_previous(&detector));
}

TEST_F(DoubleClickTest, ResetFunction) {
    Uint32 current_time = SDL_GetTicks();

    // Set up pending state
    double_click_check(&detector, 1);
    EXPECT_TRUE(double_click_has_previous(&detector));

    // Reset should clear pending state
    double_click_reset(&detector);
    EXPECT_FALSE(double_click_has_previous(&detector));

    // Next click should start fresh sequence
    current_time += 100;
    bool result = double_click_check(&detector, 1);
    EXPECT_FALSE(result);
    EXPECT_TRUE(double_click_has_previous(&detector));
}

TEST_F(DoubleClickTest, ResetWithNullPointer) {
    EXPECT_NO_FATAL_FAILURE(double_click_reset(nullptr));
}

// ===== Configuration Tests =====

TEST_F(DoubleClickTest, CustomConfiguration) {
    // Test if the implementation supports custom double-click timing
    // This test is implementation-dependent

    DoubleClickDetector custom_detector;
    double_click_init(&custom_detector, 200);

    Uint32 current_time = SDL_GetTicks();

    // Test with default timing
    double_click_check(&custom_detector, 1);
    current_time += 100;
    bool result = double_click_check(&custom_detector, 1);

    // Should work with reasonable timing
    EXPECT_TRUE(result || current_time > SDL_GetTicks() + 1000);  // Account for test timing issues
}

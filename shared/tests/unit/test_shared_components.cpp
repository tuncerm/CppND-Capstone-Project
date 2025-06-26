/**
 * Unit Tests for Core Shared Components Library
 *
 * Tests the main library initialization, version management,
 * and core functionality.
 */

#include <gtest/gtest.h>
#include <thread>
#include "../utils/test_helpers.h"
#include "shared_components.h"

class SharedComponentsTest : public ::testing::Test {
   protected:
    void SetUp() override {
        // Tests run with library already initialized by global environment
        // but we can test re-initialization behavior
    }

    void TearDown() override {
        // Ensure library is in initialized state for other tests
        if (!shared_components_is_initialized()) {
            ASSERT_TRUE(shared_components_init());
        }
    }
};

// ===== Version Tests =====

TEST_F(SharedComponentsTest, VersionStringIsValid) {
    const char* version = shared_components_get_version();
    ASSERT_NE(nullptr, version);
    EXPECT_TRUE(strlen(version) > 0);

    // Version should be in format "x.y.z"
    EXPECT_TRUE(strstr(version, ".") != nullptr) << "Version string should contain dots";

    // Should match the defined constants
    char expected_version[32];
    snprintf(expected_version, sizeof(expected_version), "%d.%d.%d",
             SHARED_COMPONENTS_VERSION_MAJOR, SHARED_COMPONENTS_VERSION_MINOR,
             SHARED_COMPONENTS_VERSION_PATCH);

    EXPECT_STREQ(expected_version, version);
}

TEST_F(SharedComponentsTest, VersionConstantsAreValid) {
    EXPECT_GE(SHARED_COMPONENTS_VERSION_MAJOR, 0);
    EXPECT_GE(SHARED_COMPONENTS_VERSION_MINOR, 0);
    EXPECT_GE(SHARED_COMPONENTS_VERSION_PATCH, 0);

    // Current version should be 1.0.0
    EXPECT_EQ(1, SHARED_COMPONENTS_VERSION_MAJOR);
    EXPECT_EQ(0, SHARED_COMPONENTS_VERSION_MINOR);
    EXPECT_EQ(0, SHARED_COMPONENTS_VERSION_PATCH);
}

// ===== Initialization Tests =====

TEST_F(SharedComponentsTest, LibraryIsInitialized) {
    // Library should be initialized by global test environment
    EXPECT_TRUE(shared_components_is_initialized());
}

TEST_F(SharedComponentsTest, InitializationIsIdempotent) {
    // Multiple calls to init should be safe
    EXPECT_TRUE(shared_components_is_initialized());
    EXPECT_TRUE(shared_components_init());
    EXPECT_TRUE(shared_components_is_initialized());
    EXPECT_TRUE(shared_components_init());
    EXPECT_TRUE(shared_components_is_initialized());
}

TEST_F(SharedComponentsTest, CleanupAndReinitialize) {
    // Test cleanup and re-initialization cycle
    EXPECT_TRUE(shared_components_is_initialized());

    shared_components_cleanup();
    EXPECT_FALSE(shared_components_is_initialized());

    // Cleanup should be safe to call multiple times
    shared_components_cleanup();
    EXPECT_FALSE(shared_components_is_initialized());

    // Re-initialize
    EXPECT_TRUE(shared_components_init());
    EXPECT_TRUE(shared_components_is_initialized());
}

// ===== Font Data Validation Tests =====

TEST_F(SharedComponentsTest, FontDataIsValid) {
    // Font data should be validated during initialization
    EXPECT_TRUE(ValidateFontData());
}

// ===== Integration State Tests =====

TEST_F(SharedComponentsTest, ComponentsCanBeUsedAfterInit) {
    ASSERT_TRUE(shared_components_is_initialized());

    // Test that we can create and use core components
    // This is a smoke test to ensure initialization properly sets up dependencies

    // Test palette manager
    PaletteManager pm;
    EXPECT_TRUE(palette_manager_init(&pm));
    RGBA color = palette_get_color(&pm, 0);
    // Should not crash and should return some valid color
    EXPECT_TRUE(color.a > 0 || color.r > 0 || color.g > 0 || color.b > 0);

    // Test font validation (called during shared_components_init)
    EXPECT_NO_FATAL_FAILURE(font_validate_data());
}

// ===== Error Handling Tests =====

TEST_F(SharedComponentsTest, HandleUninitializedState) {
    // Test behavior when library is not initialized
    shared_components_cleanup();
    EXPECT_FALSE(shared_components_is_initialized());

    // Library should handle being used in uninitialized state gracefully
    // (though specific behavior depends on individual components)

    // Version should still work
    const char* version = shared_components_get_version();
    EXPECT_NE(nullptr, version);
    EXPECT_TRUE(strlen(version) > 0);

    // Re-initialize for other tests
    EXPECT_TRUE(shared_components_init());
}

// ===== Thread Safety Tests =====

TEST_F(SharedComponentsTest, InitializationIsThreadSafe) {
    // Test that multiple threads can safely call init
    // This is a basic test - real thread safety would need more extensive testing

    EXPECT_TRUE(shared_components_is_initialized());

    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};

    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&success_count]() {
            if (shared_components_init()) {
                success_count++;
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // All threads should succeed (idempotent initialization)
    EXPECT_EQ(4, success_count.load());
    EXPECT_TRUE(shared_components_is_initialized());
}

// ===== Performance Tests =====

TEST_F(SharedComponentsTest, InitializationIsQuick) {
    // Test that initialization doesn't take too long
    shared_components_cleanup();

    auto start = std::chrono::high_resolution_clock::now();
    EXPECT_TRUE(shared_components_init());
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Initialization should complete within reasonable time (100ms is generous)
    EXPECT_LT(duration.count(), 100) << "Initialization took " << duration.count() << "ms";
}

TEST_F(SharedComponentsTest, VersionCallIsQuick) {
    // Version calls should be very fast (cached)
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000; ++i) {
        const char* version = shared_components_get_version();
        (void)version;  // Suppress unused variable warning
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // 1000 version calls should complete quickly
    EXPECT_LT(duration.count(), 1000) << "1000 version calls took " << duration.count() << "μs";
}

/**
 * Test Main for Shared Components Library
 *
 * Google Test main entry point with custom initialization for SDL3 and
 * shared components library testing.
 */

#include <SDL3/SDL.h>
#include <gtest/gtest.h>
#include "shared_components.h"
#include "utils/test_helpers.h"

class SharedComponentsTestEnvironment : public ::testing::Environment {
   public:
    void SetUp() override {
        // Initialize SDL3 for tests that need it
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            GTEST_FATAL_FAILURE_("Failed to initialize SDL3 for testing");
        }

        // Initialize shared components library
        if (!shared_components_init()) {
            SDL_Quit();
            GTEST_FATAL_FAILURE_("Failed to initialize shared components library for testing");
        }

        std::printf("=== Shared Components Test Environment Initialized ===\n");
        std::printf("SDL3 Version: %d.%d.%d\n", SDL_MAJOR_VERSION, SDL_MINOR_VERSION,
                    SDL_MICRO_VERSION);
        std::printf("Shared Components Version: %s\n", shared_components_get_version());
        std::printf("======================================================\n");
    }

    void TearDown() override {
        // Cleanup shared components
        shared_components_cleanup();

        // Cleanup SDL3
        SDL_Quit();

        std::printf("=== Shared Components Test Environment Cleaned Up ===\n");
    }
};

int main(int argc, char** argv) {
    // Initialize Google Test
    ::testing::InitGoogleTest(&argc, argv);

    // Add global test environment
    ::testing::AddGlobalTestEnvironment(new SharedComponentsTestEnvironment);

    // Configure test output
    ::testing::FLAGS_gtest_print_time = true;
    ::testing::FLAGS_gtest_print_utf8 = true;

    // Run all tests
    int result = RUN_ALL_TESTS();

    return result;
}

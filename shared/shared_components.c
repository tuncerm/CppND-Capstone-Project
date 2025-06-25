#include "shared_components.h"
#include <stdio.h>

static bool g_shared_components_initialized = false;

/**
 * Get library version string
 */
const char* shared_components_get_version(void) {
    static char version_string[32];
    snprintf(version_string, sizeof(version_string), "%d.%d.%d", SHARED_COMPONENTS_VERSION_MAJOR,
             SHARED_COMPONENTS_VERSION_MINOR, SHARED_COMPONENTS_VERSION_PATCH);
    return version_string;
}

/**
 * Initialize shared components library
 */
bool shared_components_init(void) {
    if (g_shared_components_initialized) {
        return true;  // Already initialized
    }

    // Validate font data integrity
    font_validate_data();

    g_shared_components_initialized = true;
    printf("Shared Components Library v%s initialized\n", shared_components_get_version());

    return true;
}

/**
 * Cleanup shared components library
 */
void shared_components_cleanup(void) {
    if (!g_shared_components_initialized) {
        return;
    }

    g_shared_components_initialized = false;
    printf("Shared Components Library cleaned up\n");
}

/**
 * Check if shared components library is initialized
 */
bool shared_components_is_initialized(void) {
    return g_shared_components_initialized;
}

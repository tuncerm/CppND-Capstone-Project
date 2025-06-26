#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Configuration Manager for Shared Component Library
 *
 * Centralized configuration system that replaces hardcoded values throughout
 * the codebase with JSON-based configuration files. Supports runtime validation,
 * reloading, and per-application configuration.
 */

#define CONFIG_MAX_STRING_LENGTH 256
#define CONFIG_MAX_PATH_LENGTH 512
#define CONFIG_MAX_KEYS 64
#define CONFIG_MAX_SECTIONS 16

/**
 * Configuration value types
 */
typedef enum {
    CONFIG_TYPE_INVALID = 0,
    CONFIG_TYPE_INT,
    CONFIG_TYPE_FLOAT,
    CONFIG_TYPE_BOOL,
    CONFIG_TYPE_STRING,
    CONFIG_TYPE_COLOR_RGB,
    CONFIG_TYPE_COLOR_RGBA
} ConfigValueType;

/**
 * RGB Color structure for configuration
 */
typedef struct {
    uint8_t r, g, b;
} ConfigColorRGB;

/**
 * RGBA Color structure for configuration
 */
typedef struct {
    uint8_t r, g, b, a;
} ConfigColorRGBA;

#ifdef __cplusplus
inline bool operator<(const ConfigColorRGBA& lhs, const ConfigColorRGBA& rhs) {
    if (lhs.r != rhs.r)
        return lhs.r < rhs.r;
    if (lhs.g != rhs.g)
        return lhs.g < rhs.g;
    if (lhs.b != rhs.b)
        return lhs.b < rhs.b;
    return lhs.a < rhs.a;
}

inline bool operator==(const ConfigColorRGBA& lhs, const ConfigColorRGBA& rhs) {
    return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
}
#endif

/**
 * Configuration value union
 */
typedef union {
    int int_val;
    float float_val;
    bool bool_val;
    char string_val[CONFIG_MAX_STRING_LENGTH];
    ConfigColorRGB rgb_val;
    ConfigColorRGBA rgba_val;
} ConfigValue;

/**
 * Configuration entry structure
 */
typedef struct {
    char key[CONFIG_MAX_STRING_LENGTH];
    char section[CONFIG_MAX_STRING_LENGTH];
    ConfigValueType type;
    ConfigValue value;
    ConfigValue default_value;
    bool is_required;
    bool is_valid;
} ConfigEntry;

/**
 * Configuration manager structure
 */
typedef struct {
    ConfigEntry entries[CONFIG_MAX_KEYS];
    int entry_count;
    char config_file_path[CONFIG_MAX_PATH_LENGTH];
    char application_name[CONFIG_MAX_STRING_LENGTH];
    bool is_loaded;
} ConfigManager;

// ===== Core Configuration Functions =====

/**
 * Initialize configuration manager
 *
 * @param cm Configuration manager to initialize
 * @param app_name Application name for configuration context
 * @return true if successful, false on error
 */
bool config_manager_init(ConfigManager* cm, const char* app_name);

/**
 * Load configuration from JSON file
 *
 * @param cm Configuration manager
 * @param config_path Path to JSON configuration file
 * @return true if successful, false on error
 */
bool config_manager_load(ConfigManager* cm, const char* config_path);

/**
 * Reload configuration from current file
 *
 * @param cm Configuration manager
 * @return true if successful, false on error
 */
bool config_manager_reload(ConfigManager* cm);

/**
 * Save current configuration to file
 *
 * @param cm Configuration manager
 * @param config_path Path to save configuration (NULL to use current path)
 * @return true if successful, false on error
 */
bool config_manager_save(ConfigManager* cm, const char* config_path);

/**
 * Validate all configuration entries
 *
 * @param cm Configuration manager
 * @return true if all entries are valid, false otherwise
 */
bool config_manager_validate(ConfigManager* cm);

// ===== Configuration Entry Management =====

/**
 * Register configuration entry with validation
 *
 * @param cm Configuration manager
 * @param section Configuration section name
 * @param key Configuration key name
 * @param type Value type
 * @param default_val Default value
 * @param required Whether this entry is required
 * @return true if successful, false on error
 */
bool config_register_entry(ConfigManager* cm, const char* section, const char* key,
                           ConfigValueType type, ConfigValue default_val, bool required);

/**
 * Set configuration value
 *
 * @param cm Configuration manager
 * @param section Configuration section name
 * @param key Configuration key name
 * @param value New value
 * @return true if successful, false on error
 */
bool config_set_value(ConfigManager* cm, const char* section, const char* key, ConfigValue value);

// ===== Configuration Value Getters =====

/**
 * Get integer configuration value
 *
 * @param cm Configuration manager
 * @param section Configuration section name
 * @param key Configuration key name
 * @param default_val Default value if not found
 * @return Configuration value or default
 */
int config_get_int(const ConfigManager* cm, const char* section, const char* key, int default_val);

/**
 * Get float configuration value
 *
 * @param cm Configuration manager
 * @param section Configuration section name
 * @param key Configuration key name
 * @param default_val Default value if not found
 * @return Configuration value or default
 */
float config_get_float(const ConfigManager* cm, const char* section, const char* key,
                       float default_val);

/**
 * Get boolean configuration value
 *
 * @param cm Configuration manager
 * @param section Configuration section name
 * @param key Configuration key name
 * @param default_val Default value if not found
 * @return Configuration value or default
 */
bool config_get_bool(const ConfigManager* cm, const char* section, const char* key,
                     bool default_val);

/**
 * Get string configuration value
 *
 * @param cm Configuration manager
 * @param section Configuration section name
 * @param key Configuration key name
 * @param default_val Default value if not found
 * @return Configuration value or default (do not modify returned string)
 */
const char* config_get_string(const ConfigManager* cm, const char* section, const char* key,
                              const char* default_val);

/**
 * Get RGB color configuration value
 *
 * @param cm Configuration manager
 * @param section Configuration section name
 * @param key Configuration key name
 * @param default_val Default value if not found
 * @return Configuration value or default
 */
ConfigColorRGB config_get_rgb(const ConfigManager* cm, const char* section, const char* key,
                              ConfigColorRGB default_val);

/**
 * Get RGBA color configuration value
 *
 * @param cm Configuration manager
 * @param section Configuration section name
 * @param key Configuration key name
 * @param default_val Default value if not found
 * @return Configuration value or default
 */
ConfigColorRGBA config_get_rgba(const ConfigManager* cm, const char* section, const char* key,
                                ConfigColorRGBA default_val);

// ===== Utility Functions =====

/**
 * Create ConfigValue from integer
 *
 * @param val Integer value
 * @return ConfigValue structure
 */
ConfigValue config_make_int(int val);

/**
 * Create ConfigValue from float
 *
 * @param val Float value
 * @return ConfigValue structure
 */
ConfigValue config_make_float(float val);

/**
 * Create ConfigValue from boolean
 *
 * @param val Boolean value
 * @return ConfigValue structure
 */
ConfigValue config_make_bool(bool val);

/**
 * Create ConfigValue from string
 *
 * @param val String value
 * @return ConfigValue structure
 */
ConfigValue config_make_string(const char* val);

/**
 * Create ConfigValue from RGB color
 *
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @return ConfigValue structure
 */
ConfigValue config_make_rgb(uint8_t r, uint8_t g, uint8_t b);

/**
 * Create ConfigValue from RGBA color
 *
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @param a Alpha component (0-255)
 * @return ConfigValue structure
 */
ConfigValue config_make_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/**
 * Check if configuration entry exists
 *
 * @param cm Configuration manager
 * @param section Configuration section name
 * @param key Configuration key name
 * @return true if entry exists, false otherwise
 */
bool config_has_entry(const ConfigManager* cm, const char* section, const char* key);

/**
 * Print configuration summary for debugging
 *
 * @param cm Configuration manager
 */
void config_print_summary(const ConfigManager* cm);

#ifdef __cplusplus
}
#endif

#endif  // CONFIG_MANAGER_H

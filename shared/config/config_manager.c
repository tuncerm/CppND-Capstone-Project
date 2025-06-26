#include "config_manager.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../error_handler/error_handler.h"

/**
 * Simple JSON parser for configuration files
 * Supports basic JSON structure with strings, numbers, booleans, and objects
 */

// Forward declarations for JSON parsing
static bool parse_json_file(ConfigManager* cm, const char* filepath);
static bool parse_json_object(ConfigManager* cm, const char* json, size_t* pos,
                              const char* section);
static bool parse_json_value(ConfigManager* cm, const char* json, size_t* pos, const char* section,
                             const char* key);
static bool write_json_file(const ConfigManager* cm, const char* filepath);
static void skip_whitespace(const char* json, size_t* pos);
static bool parse_string(const char* json, size_t* pos, char* output, size_t max_len);
static bool parse_number(const char* json, size_t* pos, double* output);
static bool parse_boolean(const char* json, size_t* pos, bool* output);
static bool parse_color_string(const char* color_str, ConfigColorRGBA* rgba);

// Internal helper functions
static ConfigEntry* find_entry(ConfigManager* cm, const char* section, const char* key);
static bool validate_entry_value(const ConfigEntry* entry);

// ===== Core Configuration Functions =====

bool config_manager_init(ConfigManager* cm, const char* app_name) {
    if (!cm || !app_name) {
        return false;
    }

    memset(cm, 0, sizeof(ConfigManager));
    strncpy(cm->application_name, app_name, CONFIG_MAX_STRING_LENGTH - 1);
    cm->application_name[CONFIG_MAX_STRING_LENGTH - 1] = '\0';

    return true;
}

bool config_manager_load(ConfigManager* cm, const char* config_path) {
    if (!cm || !config_path) {
        ErrorHandler_Set(ERR_INVALID_ARGUMENT, __FILE__, __LINE__,
                         "Invalid parameters for config_manager_load");
        return false;
    }

    strncpy(cm->config_file_path, config_path, CONFIG_MAX_PATH_LENGTH - 1);
    cm->config_file_path[CONFIG_MAX_PATH_LENGTH - 1] = '\0';

    if (!parse_json_file(cm, config_path)) {
        return false;
    }

    cm->is_loaded = true;
    return config_manager_validate(cm);
}

bool config_manager_reload(ConfigManager* cm) {
    if (!cm || !cm->is_loaded || strlen(cm->config_file_path) == 0) {
        ErrorHandler_Set(ERR_INVALID_STATE, __FILE__, __LINE__,
                         "No configuration file loaded to reload");
        return false;
    }

    return config_manager_load(cm, cm->config_file_path);
}

bool config_manager_save(ConfigManager* cm, const char* config_path) {
    if (!cm) {
        return false;
    }

    const char* save_path = config_path ? config_path : cm->config_file_path;
    if (!save_path || strlen(save_path) == 0) {
        ErrorHandler_Set(ERR_INVALID_ARGUMENT, __FILE__, __LINE__,
                         "No configuration file path specified for save");
        return false;
    }

    return write_json_file(cm, save_path);
}

bool config_manager_validate(ConfigManager* cm) {
    if (!cm) {
        return false;
    }

    bool all_valid = true;

    for (int i = 0; i < cm->entry_count; i++) {
        ConfigEntry* entry = &cm->entries[i];

        if (entry->is_required && !entry->is_valid) {
            ErrorHandler_Set(ERR_CONFIG_PARSE, __FILE__, __LINE__,
                             "Required configuration entry missing: [%s]%s", entry->section,
                             entry->key);
            all_valid = false;
        }

        if (!validate_entry_value(entry)) {
            ErrorHandler_Set(ERR_CONFIG_PARSE, __FILE__, __LINE__,
                             "Invalid value for configuration entry: [%s]%s", entry->section,
                             entry->key);
            all_valid = false;
        }
    }

    return all_valid;
}

// ===== Configuration Entry Management =====

bool config_register_entry(ConfigManager* cm, const char* section, const char* key,
                           ConfigValueType type, ConfigValue default_val, bool required) {
    if (!cm || !section || !key || cm->entry_count >= CONFIG_MAX_KEYS) {
        ErrorHandler_Set(
            ERR_INVALID_ARGUMENT, __FILE__, __LINE__,
            "Cannot register configuration entry (invalid params or max keys reached)");
        return false;
    }

    ConfigEntry* entry = &cm->entries[cm->entry_count];
    strncpy(entry->section, section, CONFIG_MAX_STRING_LENGTH - 1);
    strncpy(entry->key, key, CONFIG_MAX_STRING_LENGTH - 1);
    entry->section[CONFIG_MAX_STRING_LENGTH - 1] = '\0';
    entry->key[CONFIG_MAX_STRING_LENGTH - 1] = '\0';

    entry->type = type;
    entry->default_value = default_val;
    entry->value = default_val;
    entry->is_required = required;
    entry->is_valid = !required;  // Not required entries are valid by default

    cm->entry_count++;
    return true;
}

bool config_set_value(ConfigManager* cm, const char* section, const char* key, ConfigValue value) {
    if (!cm || !section || !key) {
        return false;
    }

    ConfigEntry* entry = find_entry(cm, section, key);
    if (!entry) {
        return false;
    }

    entry->value = value;
    entry->is_valid = true;
    return true;
}

// ===== Configuration Value Getters =====

int config_get_int(const ConfigManager* cm, const char* section, const char* key, int default_val) {
    if (!cm || !section || !key) {
        return default_val;
    }

    ConfigEntry* entry = find_entry((ConfigManager*)cm, section, key);
    if (!entry || entry->type != CONFIG_TYPE_INT || !entry->is_valid) {
        return default_val;
    }

    return entry->value.int_val;
}

float config_get_float(const ConfigManager* cm, const char* section, const char* key,
                       float default_val) {
    if (!cm || !section || !key) {
        return default_val;
    }

    ConfigEntry* entry = find_entry((ConfigManager*)cm, section, key);
    if (!entry || entry->type != CONFIG_TYPE_FLOAT || !entry->is_valid) {
        return default_val;
    }

    return entry->value.float_val;
}

bool config_get_bool(const ConfigManager* cm, const char* section, const char* key,
                     bool default_val) {
    if (!cm || !section || !key) {
        return default_val;
    }

    ConfigEntry* entry = find_entry((ConfigManager*)cm, section, key);
    if (!entry || entry->type != CONFIG_TYPE_BOOL || !entry->is_valid) {
        return default_val;
    }

    return entry->value.bool_val;
}

const char* config_get_string(const ConfigManager* cm, const char* section, const char* key,
                              const char* default_val) {
    if (!cm || !section || !key) {
        return default_val;
    }

    ConfigEntry* entry = find_entry((ConfigManager*)cm, section, key);
    if (!entry || entry->type != CONFIG_TYPE_STRING || !entry->is_valid) {
        return default_val;
    }

    return entry->value.string_val;
}

ConfigColorRGB config_get_rgb(const ConfigManager* cm, const char* section, const char* key,
                              ConfigColorRGB default_val) {
    if (!cm || !section || !key) {
        return default_val;
    }

    ConfigEntry* entry = find_entry((ConfigManager*)cm, section, key);
    if (!entry || entry->type != CONFIG_TYPE_COLOR_RGB || !entry->is_valid) {
        return default_val;
    }

    return entry->value.rgb_val;
}

ConfigColorRGBA config_get_rgba(const ConfigManager* cm, const char* section, const char* key,
                                ConfigColorRGBA default_val) {
    if (!cm || !section || !key) {
        return default_val;
    }

    ConfigEntry* entry = find_entry((ConfigManager*)cm, section, key);
    if (!entry || entry->type != CONFIG_TYPE_COLOR_RGBA || !entry->is_valid) {
        return default_val;
    }

    return entry->value.rgba_val;
}

// ===== Utility Functions =====

ConfigValue config_make_int(int val) {
    ConfigValue cv;
    memset(&cv, 0, sizeof(cv));
    cv.int_val = val;
    return cv;
}

ConfigValue config_make_float(float val) {
    ConfigValue cv;
    memset(&cv, 0, sizeof(cv));
    cv.float_val = val;
    return cv;
}

ConfigValue config_make_bool(bool val) {
    ConfigValue cv;
    memset(&cv, 0, sizeof(cv));
    cv.bool_val = val;
    return cv;
}

ConfigValue config_make_string(const char* val) {
    ConfigValue cv;
    memset(&cv, 0, sizeof(cv));
    if (val) {
        strncpy(cv.string_val, val, CONFIG_MAX_STRING_LENGTH - 1);
        cv.string_val[CONFIG_MAX_STRING_LENGTH - 1] = '\0';
    }
    return cv;
}

ConfigValue config_make_rgb(uint8_t r, uint8_t g, uint8_t b) {
    ConfigValue cv;
    memset(&cv, 0, sizeof(cv));
    cv.rgb_val.r = r;
    cv.rgb_val.g = g;
    cv.rgb_val.b = b;
    return cv;
}

ConfigValue config_make_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    ConfigValue cv;
    memset(&cv, 0, sizeof(cv));
    cv.rgba_val.r = r;
    cv.rgba_val.g = g;
    cv.rgba_val.b = b;
    cv.rgba_val.a = a;
    return cv;
}

bool config_has_entry(const ConfigManager* cm, const char* section, const char* key) {
    if (!cm || !section || !key) {
        return false;
    }

    return find_entry((ConfigManager*)cm, section, key) != NULL;
}

void config_print_summary(const ConfigManager* cm) {
    if (!cm) {
        printf("Invalid configuration manager\n");
        return;
    }

    printf("Configuration Summary for %s:\n", cm->application_name);
    printf("  Config file: %s\n", cm->config_file_path);
    printf("  Loaded: %s\n", cm->is_loaded ? "Yes" : "No");
    printf("  Entry count: %d\n", cm->entry_count);

    if (ErrorHandler_HasError()) {
        printf("  Last error: ");
        ErrorHandler_Log();
    }

    printf("  Entries:\n");
    for (int i = 0; i < cm->entry_count; i++) {
        const ConfigEntry* entry = &cm->entries[i];
        printf("    [%s]%s (%s) - %s\n", entry->section, entry->key,
               entry->is_required ? "required" : "optional", entry->is_valid ? "valid" : "invalid");
    }
}

// ===== Internal Helper Functions =====

static ConfigEntry* find_entry(ConfigManager* cm, const char* section, const char* key) {
    if (!cm || !section || !key) {
        return NULL;
    }

    for (int i = 0; i < cm->entry_count; i++) {
        ConfigEntry* entry = &cm->entries[i];
        if (strcmp(entry->section, section) == 0 && strcmp(entry->key, key) == 0) {
            return entry;
        }
    }

    return NULL;
}

static bool validate_entry_value(const ConfigEntry* entry) {
    if (!entry) {
        return false;
    }

    switch (entry->type) {
        case CONFIG_TYPE_STRING:
            return strlen(entry->value.string_val) > 0;
        case CONFIG_TYPE_COLOR_RGB:
            // RGB values are always valid (0-255 range enforced by uint8_t)
            return true;
        case CONFIG_TYPE_COLOR_RGBA:
            // RGBA values are always valid (0-255 range enforced by uint8_t)
            return true;
        case CONFIG_TYPE_INT:
        case CONFIG_TYPE_FLOAT:
        case CONFIG_TYPE_BOOL:
            return true;
        default:
            return false;
    }
}

// ===== JSON Parser Implementation =====

static bool parse_json_file(ConfigManager* cm, const char* filepath) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        ErrorHandler_Set(ERR_FILE_OPEN, __FILE__, __LINE__, "Cannot open configuration file: %s",
                         filepath);
        return false;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size <= 0) {
        fclose(file);
        ErrorHandler_Set(ERR_FILE_READ, __FILE__, __LINE__,
                         "Configuration file is empty or read error: %s", filepath);
        return false;
    }

    // Read file content
    char* json_content = (char*)malloc(file_size + 1);
    if (!json_content) {
        fclose(file);
        ErrorHandler_Set(ERR_MEMORY_ALLOC, __FILE__, __LINE__,
                         "Failed to allocate memory for config file content");
        return false;
    }

    size_t read_size = fread(json_content, 1, file_size, file);
    json_content[read_size] = '\0';
    fclose(file);

    // Parse JSON
    size_t pos = 0;
    bool result = parse_json_object(cm, json_content, &pos, "");

    free(json_content);
    return result;
}

static bool parse_json_object(ConfigManager* cm, const char* json, size_t* pos,
                              const char* section) {
    skip_whitespace(json, pos);

    if (json[*pos] != '{') {
        ErrorHandler_Set(ERR_CONFIG_PARSE, __FILE__, __LINE__,
                         "Expected '{' at start of JSON object");
        return false;
    }
    (*pos)++;

    while (json[*pos] != '\0') {
        skip_whitespace(json, pos);

        if (json[*pos] == '}') {
            (*pos)++;
            return true;
        }

        // Parse key
        char key[CONFIG_MAX_STRING_LENGTH];
        if (!parse_string(json, pos, key, sizeof(key))) {
            ErrorHandler_Set(ERR_CONFIG_PARSE, __FILE__, __LINE__, "Failed to parse JSON key");
            return false;
        }

        skip_whitespace(json, pos);
        if (json[*pos] != ':') {
            ErrorHandler_Set(ERR_CONFIG_PARSE, __FILE__, __LINE__, "Expected ':' after JSON key");
            return false;
        }
        (*pos)++;

        // Parse value
        skip_whitespace(json, pos);
        if (json[*pos] == '{') {
            // Nested object - use key as section name
            char new_section[CONFIG_MAX_STRING_LENGTH];
            if (strlen(section) > 0) {
                snprintf(new_section, sizeof(new_section), "%s.%s", section, key);
            } else {
                strncpy(new_section, key, sizeof(new_section) - 1);
                new_section[sizeof(new_section) - 1] = '\0';
            }

            if (!parse_json_object(cm, json, pos, new_section)) {
                return false;
            }
        } else {
            // Regular value
            if (!parse_json_value(cm, json, pos, section, key)) {
                return false;
            }
        }

        skip_whitespace(json, pos);
        if (json[*pos] == ',') {
            (*pos)++;
        } else if (json[*pos] != '}') {
            ErrorHandler_Set(ERR_CONFIG_PARSE, __FILE__, __LINE__,
                             "Expected ',' or '}' in JSON object");
            return false;
        }
    }

    ErrorHandler_Set(ERR_CONFIG_PARSE, __FILE__, __LINE__, "Unexpected end of JSON");
    return false;
}

static bool parse_json_value(ConfigManager* cm, const char* json, size_t* pos, const char* section,
                             const char* key) {
    skip_whitespace(json, pos);

    ConfigEntry* entry = find_entry(cm, section, key);
    if (!entry) {
        // Skip unknown values
        if (json[*pos] == '"') {
            char temp[CONFIG_MAX_STRING_LENGTH];
            return parse_string(json, pos, temp, sizeof(temp));
        } else if (isdigit(json[*pos]) || json[*pos] == '-' || json[*pos] == '+') {
            double temp;
            return parse_number(json, pos, &temp);
        } else if (strncmp(&json[*pos], "true", 4) == 0 || strncmp(&json[*pos], "false", 5) == 0) {
            bool temp;
            return parse_boolean(json, pos, &temp);
        }
        return false;
    }

    ConfigValue value;
    memset(&value, 0, sizeof(value));

    switch (entry->type) {
        case CONFIG_TYPE_STRING: {
            if (!parse_string(json, pos, value.string_val, sizeof(value.string_val))) {
                return false;
            }
            break;
        }
        case CONFIG_TYPE_INT: {
            double temp;
            if (!parse_number(json, pos, &temp)) {
                return false;
            }
            value.int_val = (int)temp;
            break;
        }
        case CONFIG_TYPE_FLOAT: {
            double temp;
            if (!parse_number(json, pos, &temp)) {
                return false;
            }
            value.float_val = (float)temp;
            break;
        }
        case CONFIG_TYPE_BOOL: {
            if (!parse_boolean(json, pos, &value.bool_val)) {
                return false;
            }
            break;
        }
        case CONFIG_TYPE_COLOR_RGB:
        case CONFIG_TYPE_COLOR_RGBA: {
            char color_str[CONFIG_MAX_STRING_LENGTH];
            if (!parse_string(json, pos, color_str, sizeof(color_str))) {
                return false;
            }

            ConfigColorRGBA rgba;
            if (!parse_color_string(color_str, &rgba)) {
                return false;
            }

            if (entry->type == CONFIG_TYPE_COLOR_RGB) {
                value.rgb_val.r = rgba.r;
                value.rgb_val.g = rgba.g;
                value.rgb_val.b = rgba.b;
            } else {
                value.rgba_val = rgba;
            }
            break;
        }
        default:
            return false;
    }

    entry->value = value;
    entry->is_valid = true;
    return true;
}

static void skip_whitespace(const char* json, size_t* pos) {
    while (json[*pos] != '\0' && isspace(json[*pos])) {
        (*pos)++;
    }
}

static bool parse_string(const char* json, size_t* pos, char* output, size_t max_len) {
    skip_whitespace(json, pos);

    if (json[*pos] != '"') {
        return false;
    }
    (*pos)++;

    size_t out_pos = 0;
    while (json[*pos] != '\0' && json[*pos] != '"' && out_pos < max_len - 1) {
        if (json[*pos] == '\\' && json[*pos + 1] != '\0') {
            (*pos)++;
            switch (json[*pos]) {
                case 'n':
                    output[out_pos++] = '\n';
                    break;
                case 't':
                    output[out_pos++] = '\t';
                    break;
                case 'r':
                    output[out_pos++] = '\r';
                    break;
                case '\\':
                    output[out_pos++] = '\\';
                    break;
                case '"':
                    output[out_pos++] = '"';
                    break;
                default:
                    output[out_pos++] = json[*pos];
                    break;
            }
        } else {
            output[out_pos++] = json[*pos];
        }
        (*pos)++;
    }

    if (json[*pos] != '"') {
        return false;
    }
    (*pos)++;

    output[out_pos] = '\0';
    return true;
}

static bool parse_number(const char* json, size_t* pos, double* output) {
    skip_whitespace(json, pos);

    char* end_ptr;
    *output = strtod(&json[*pos], &end_ptr);

    if (end_ptr == &json[*pos]) {
        return false;
    }

    *pos = end_ptr - json;
    return true;
}

static bool parse_boolean(const char* json, size_t* pos, bool* output) {
    skip_whitespace(json, pos);

    if (strncmp(&json[*pos], "true", 4) == 0) {
        *output = true;
        *pos += 4;
        return true;
    } else if (strncmp(&json[*pos], "false", 5) == 0) {
        *output = false;
        *pos += 5;
        return true;
    }

    return false;
}

static bool parse_color_string(const char* color_str, ConfigColorRGBA* rgba) {
    if (!color_str || !rgba) {
        return false;
    }

    // Support hex format: #RRGGBB or #RRGGBBAA
    if (color_str[0] == '#') {
        size_t len = strlen(color_str);
        if (len == 7) {  // #RRGGBB
            unsigned int r, g, b;
            if (sscanf(color_str + 1, "%02x%02x%02x", &r, &g, &b) == 3) {
                rgba->r = (uint8_t)r;
                rgba->g = (uint8_t)g;
                rgba->b = (uint8_t)b;
                rgba->a = 255;
                return true;
            }
        } else if (len == 9) {  // #RRGGBBAA
            unsigned int r, g, b, a;
            if (sscanf(color_str + 1, "%02x%02x%02x%02x", &r, &g, &b, &a) == 4) {
                rgba->r = (uint8_t)r;
                rgba->g = (uint8_t)g;
                rgba->b = (uint8_t)b;
                rgba->a = (uint8_t)a;
                return true;
            }
        }
    }

    // Support RGB format: rgb(r,g,b) or rgba(r,g,b,a)
    if (strncmp(color_str, "rgb(", 4) == 0) {
        unsigned int r, g, b;
        if (sscanf(color_str + 4, "%u,%u,%u)", &r, &g, &b) == 3) {
            rgba->r = (uint8_t)(r > 255 ? 255 : r);
            rgba->g = (uint8_t)(g > 255 ? 255 : g);
            rgba->b = (uint8_t)(b > 255 ? 255 : b);
            rgba->a = 255;
            return true;
        }
    } else if (strncmp(color_str, "rgba(", 5) == 0) {
        unsigned int r, g, b, a;
        if (sscanf(color_str + 5, "%u,%u,%u,%u)", &r, &g, &b, &a) == 4) {
            rgba->r = (uint8_t)(r > 255 ? 255 : r);
            rgba->g = (uint8_t)(g > 255 ? 255 : g);
            rgba->b = (uint8_t)(b > 255 ? 255 : b);
            rgba->a = (uint8_t)(a > 255 ? 255 : a);
            return true;
        }
    }

    return false;
}

static bool write_json_file(const ConfigManager* cm, const char* filepath) {
    if (!cm || !filepath) {
        return false;
    }

    FILE* file = fopen(filepath, "w");
    if (!file) {
        return false;
    }

    fprintf(file, "{\n");
    fprintf(file, "  \"_meta\": {\n");
    fprintf(file, "    \"application\": \"%s\",\n", cm->application_name);
    fprintf(file, "    \"generated_by\": \"Configuration Manager\"\n");
    fprintf(file, "  }");

    // Group entries by section
    char current_section[CONFIG_MAX_STRING_LENGTH] = "";
    bool section_open = false;

    for (int i = 0; i < cm->entry_count; i++) {
        const ConfigEntry* entry = &cm->entries[i];

        if (strcmp(current_section, entry->section) != 0) {
            // Close previous section
            if (section_open) {
                fprintf(file, "\n  }");
            }

            // Open new section
            fprintf(file, ",\n  \"%s\": {", entry->section);
            strncpy(current_section, entry->section, sizeof(current_section) - 1);
            current_section[sizeof(current_section) - 1] = '\0';
            section_open = true;
        } else {
            fprintf(file, ",");
        }

        fprintf(file, "\n    \"%s\": ", entry->key);

        switch (entry->type) {
            case CONFIG_TYPE_INT:
                fprintf(file, "%d", entry->value.int_val);
                break;
            case CONFIG_TYPE_FLOAT:
                fprintf(file, "%.6f", entry->value.float_val);
                break;
            case CONFIG_TYPE_BOOL:
                fprintf(file, "%s", entry->value.bool_val ? "true" : "false");
                break;
            case CONFIG_TYPE_STRING:
                fprintf(file, "\"%s\"", entry->value.string_val);
                break;
            case CONFIG_TYPE_COLOR_RGB:
                fprintf(file, "\"#%02x%02x%02x\"", entry->value.rgb_val.r, entry->value.rgb_val.g,
                        entry->value.rgb_val.b);
                break;
            case CONFIG_TYPE_COLOR_RGBA:
                fprintf(file, "\"#%02x%02x%02x%02x\"", entry->value.rgba_val.r,
                        entry->value.rgba_val.g, entry->value.rgba_val.b, entry->value.rgba_val.a);
                break;
            default:
                fprintf(file, "null");
                break;
        }
    }

    if (section_open) {
        fprintf(file, "\n  }");
    }

    fprintf(file, "\n}\n");
    fclose(file);
    return true;
}

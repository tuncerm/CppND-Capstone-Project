# Configuration Manager

A centralized configuration system for the CppND Capstone Project that replaces hardcoded values throughout the codebase with JSON-based configuration files.

## Features

- **JSON-based Configuration**: Human-readable configuration files with support for nested sections
- **Type Safety**: Strongly typed configuration values (int, float, bool, string, RGB, RGBA)
- **Runtime Validation**: Automatic validation of required configuration entries
- **Default Values**: Graceful fallback to defaults when configuration is missing
- **Runtime Reloading**: Configuration can be reloaded without restarting the application
- **Color Support**: Built-in parsing for hex colors (#RRGGBB, #RRGGBBAA) and rgb()/rgba() formats

## Usage

### 1. Initialize Configuration Manager

```c
#include "config/config_manager.h"

ConfigManager config;
config_manager_init(&config, "Your Application Name");
```

### 2. Register Configuration Entries

```c
// Register various types of configuration entries
config_register_entry(&config, "display", "width", CONFIG_TYPE_INT, 
                     config_make_int(800), true);  // required
config_register_entry(&config, "display", "height", CONFIG_TYPE_INT, 
                     config_make_int(600), true);  // required
config_register_entry(&config, "display", "fullscreen", CONFIG_TYPE_BOOL, 
                     config_make_bool(false), false);  // optional
config_register_entry(&config, "colors", "background", CONFIG_TYPE_COLOR_RGBA, 
                     config_make_rgba(255, 255, 255, 255), false);
```

### 3. Load Configuration File

```c
if (!config_manager_load(&config, "path/to/config.json")) {
    printf("Warning: Failed to load configuration: %s\n", 
           config_manager_get_error(&config));
    // Application will use default values
}
```

### 4. Get Configuration Values

```c
// Get values with automatic fallback to defaults
int width = config_get_int(&config, "display", "width", 800);
int height = config_get_int(&config, "display", "height", 600);
bool fullscreen = config_get_bool(&config, "display", "fullscreen", false);
const char* title = config_get_string(&config, "display", "title", "Default Title");

// Get color values
ConfigColorRGBA bg_color = config_get_rgba(&config, "colors", "background", 
                                          {255, 255, 255, 255});
```

## Configuration File Format

### Basic Structure

```json
{
  "_meta": {
    "application": "Your Application",
    "description": "Configuration file description",
    "version": "1.0"
  },
  "section_name": {
    "key_name": "value",
    "another_key": 123,
    "boolean_key": true
  },
  "colors": {
    "background": "#FF0000FF",
    "foreground": "rgba(0,255,0,255)"
  }
}
```

### Supported Value Types

- **Integers**: `"width": 1024`
- **Floats**: `"quality": 0.8`
- **Booleans**: `"enabled": true`
- **Strings**: `"title": "My Application"`
- **Colors**: 
  - Hex format: `"#RRGGBB"` or `"#RRGGBBAA"`
  - RGB format: `"rgb(255,128,64)"`
  - RGBA format: `"rgba(255,128,64,192)"`

### Example Configuration Files

#### Game Configuration (`config/game_config.json`)

```json
{
  "_meta": {
    "application": "Character Game",
    "description": "Main game application configuration",
    "version": "1.0"
  },
  "display": {
    "grid_size": 32,
    "grid_width": 32,
    "grid_height": 20,
    "window_title": "Character Game"
  },
  "performance": {
    "target_fps": 60,
    "ms_per_frame": 16
  },
  "colors": {
    "wall_color": "#FF0000FF",
    "floor_color": "#0000FFFF",
    "player_color": "#000000FF",
    "enemy_color": "#AAAA00FF"
  }
}
```

#### Tool Configuration (`config/tile_maker_config.json`)

```json
{
  "_meta": {
    "application": "Tile Maker",
    "description": "Tile editor application configuration",
    "version": "1.0"
  },
  "display": {
    "window_width": 900,
    "window_height": 600,
    "window_title": "Tile Maker v1.0 - SDL3 Edition"
  },
  "ui": {
    "palette_bar_height": 50,
    "button_width": 80,
    "button_height": 30
  },
  "performance": {
    "target_fps": 60,
    "frame_delay_ms": 16
  }
}
```

## API Reference

### Core Functions

- `config_manager_init(cm, app_name)` - Initialize configuration manager
- `config_manager_load(cm, path)` - Load configuration from JSON file
- `config_manager_reload(cm)` - Reload current configuration file
- `config_manager_save(cm, path)` - Save configuration to JSON file
- `config_manager_validate(cm)` - Validate all configuration entries

### Entry Management

- `config_register_entry(cm, section, key, type, default_val, required)` - Register configuration entry
- `config_set_value(cm, section, key, value)` - Set configuration value programmatically

### Value Getters

- `config_get_int(cm, section, key, default_val)` - Get integer value
- `config_get_float(cm, section, key, default_val)` - Get float value
- `config_get_bool(cm, section, key, default_val)` - Get boolean value
- `config_get_string(cm, section, key, default_val)` - Get string value
- `config_get_rgb(cm, section, key, default_val)` - Get RGB color value
- `config_get_rgba(cm, section, key, default_val)` - Get RGBA color value

### Utility Functions

- `config_make_int(val)` - Create integer ConfigValue
- `config_make_float(val)` - Create float ConfigValue
- `config_make_bool(val)` - Create boolean ConfigValue
- `config_make_string(val)` - Create string ConfigValue
- `config_make_rgb(r, g, b)` - Create RGB color ConfigValue
- `config_make_rgba(r, g, b, a)` - Create RGBA color ConfigValue
- `config_has_entry(cm, section, key)` - Check if entry exists
- `config_print_summary(cm)` - Print configuration summary for debugging

## Error Handling

The configuration system provides robust error handling:

```c
if (!config_manager_load(&config, "config.json")) {
    const char* error = config_manager_get_error(&config);
    printf("Configuration error: %s\n", error);
    // Application continues with default values
}

// Validate all required entries are present
if (!config_manager_validate(&config)) {
    printf("Configuration validation failed: %s\n", 
           config_manager_get_error(&config));
}
```

## Integration Examples

### Replacing Hardcoded Values

**Before:**
```c
constexpr int kGridSize{32};
constexpr int kGridWidth{32};
constexpr int kGridHeight{20};
```

**After:**
```c
ConfigManager config;
config_manager_init(&config, "Character Game");
config_register_entry(&config, "display", "grid_size", CONFIG_TYPE_INT, config_make_int(32), true);
config_register_entry(&config, "display", "grid_width", CONFIG_TYPE_INT, config_make_int(32), true);
config_register_entry(&config, "display", "grid_height", CONFIG_TYPE_INT, config_make_int(20), true);
config_manager_load(&config, "config/game_config.json");

const int kGridSize = config_get_int(&config, "display", "grid_size", 32);
const int kGridWidth = config_get_int(&config, "display", "grid_width", 32);
const int kGridHeight = config_get_int(&config, "display", "grid_height", 20);
```

### Color Configuration

**Before:**
```c
SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);  // Red wall
```

**After:**
```c
ConfigColorRGBA wall_color = config_get_rgba(&config, "colors", "wall_color", {255, 0, 0, 255});
SDL_SetRenderDrawColor(renderer, wall_color.r, wall_color.g, wall_color.b, wall_color.a);
```

## Testing

The configuration system includes comprehensive unit tests:

```bash
# Run configuration system tests
cd build
make test_config_manager
./shared/tests/test_config_manager
```

## Best Practices

1. **Register all entries before loading** - Ensure all configuration entries are registered before calling `config_manager_load()`
2. **Provide meaningful defaults** - Always provide sensible default values for configuration entries
3. **Use sections for organization** - Group related configuration values into logical sections
4. **Mark critical values as required** - Use the `required` parameter for essential configuration values
5. **Handle errors gracefully** - Always check return values and provide fallback behavior
6. **Validate after loading** - Call `config_manager_validate()` after loading to ensure all required values are present

## Migration Guide

To migrate existing applications to use the configuration system:

1. **Identify hardcoded values** - Find all magic numbers, colors, and strings in your code
2. **Create configuration file** - Organize values into logical sections in a JSON file
3. **Register configuration entries** - Add `config_register_entry()` calls for each value
4. **Replace hardcoded values** - Replace direct values with `config_get_*()` function calls
5. **Test thoroughly** - Ensure the application works with both the configuration file and default values

## Thread Safety

The current implementation is **not thread-safe**. If you need to access configuration from multiple threads, implement appropriate locking mechanisms around configuration access.

## Limitations

- Maximum of 64 configuration entries per manager
- Maximum string length of 256 characters
- Simple JSON parser (does not support all JSON features)
- No support for arrays or complex nested objects
- No automatic type conversion between similar types

## Future Enhancements

- Array support for configuration values
- Hot-reloading with file system monitoring
- Configuration value change callbacks
- Enhanced JSON parser with full JSON specification support
- Thread-safe operations
- Configuration value validation rules (min/max ranges, etc.)

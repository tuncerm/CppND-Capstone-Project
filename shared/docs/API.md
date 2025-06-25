# Shared Components Library - API Reference

Complete API documentation for all shared component library functions, structures, and constants.

## Table of Contents

- [Library Core](#library-core)
- [Text Renderer](#text-renderer)
- [Font Data](#font-data)
- [UI Framework](#ui-framework)
  - [UI Buttons](#ui-buttons)
  - [UI Primitives](#ui-primitives)
- [Palette Manager](#palette-manager)
- [SDL Framework](#sdl-framework)
- [Utilities](#utilities)
  - [Double-Click Detection](#double-click-detection)
  - [File Utils](#file-utils)

---

## Library Core

### `shared_components.h`

Main library header that includes all component headers.

#### Functions

##### `shared_components_init()`
```c
bool shared_components_init(void);
```
Initialize the shared components library. Must be called before using any shared components.

**Returns**: `true` if successful, `false` on error

**Example**:
```c
if (!shared_components_init()) {
    fprintf(stderr, "Failed to initialize shared components\n");
    exit(1);
}
```

##### `shared_components_cleanup()`
```c
void shared_components_cleanup(void);
```
Cleanup shared components library. Call when shutting down application.

##### `shared_components_is_initialized()`
```c
bool shared_components_is_initialized(void);
```
Check if shared components library is initialized.

**Returns**: `true` if initialized and ready to use

##### `shared_components_get_version()`
```c
const char* shared_components_get_version(void);
```
Get library version string in format "major.minor.patch".

**Returns**: Version string (e.g., "1.0.0")

---

## Text Renderer

### `text_renderer.h`

Text rendering system using 5x7 bitmap font with 7-segment display support.

#### Structures

##### `TextRenderer`
```c
typedef struct {
    SDL_Renderer* renderer;
    bool initialized;
    SDL_Color default_color;
} TextRenderer;
```
Text renderer context structure holding SDL renderer reference and state.

##### `SevenSegmentFlags`
```c
typedef enum {
    SEGMENT_A = 0x01,  // Top
    SEGMENT_B = 0x02,  // Top right
    SEGMENT_C = 0x04,  // Bottom right
    SEGMENT_D = 0x08,  // Bottom
    SEGMENT_E = 0x10,  // Bottom left
    SEGMENT_F = 0x20,  // Top left
    SEGMENT_G = 0x40   // Middle
} SevenSegmentFlags;
```
7-segment display segment identifiers for custom digit patterns.

#### Core Functions

##### `text_renderer_init()`
```c
bool text_renderer_init(TextRenderer* tr, SDL_Renderer* renderer);
```
Initialize text renderer with SDL renderer.

**Parameters**:
- `tr`: Text renderer context to initialize
- `renderer`: SDL renderer to use for drawing

**Returns**: `true` if successful, `false` on error

**Example**:
```c
TextRenderer text_renderer;
if (!text_renderer_init(&text_renderer, sdl_renderer)) {
    // Handle error
}
```

##### `text_renderer_cleanup()`
```c
void text_renderer_cleanup(TextRenderer* tr);
```
Cleanup text renderer resources.

##### `text_render_string()`
```c
void text_render_string(TextRenderer* tr, const char* text, int x, int y, SDL_Color color);
```
Render text string at specified position with color.

**Parameters**:
- `tr`: Text renderer context
- `text`: Text string to render (max 32 characters)
- `x`, `y`: Position coordinates
- `color`: Text color

**Example**:
```c
SDL_Color white = {255, 255, 255, 255};
text_render_string(&text_renderer, "Score: 1234", 10, 10, white);
```

##### `text_render_string_default()`
```c
void text_render_string_default(TextRenderer* tr, const char* text, int x, int y);
```
Render text string using default color set with [`text_renderer_set_default_color()`](#text_renderer_set_default_color).

##### `text_get_dimensions()`
```c
void text_get_dimensions(const char* text, int* width, int* height);
```
Calculate text dimensions for layout calculations.

**Parameters**:
- `text`: Text string to measure
- `width`: Output pointer for calculated width in pixels
- `height`: Output pointer for calculated height in pixels (always 7)

**Example**:
```c
int text_width, text_height;
text_get_dimensions("Hello", &text_width, &text_height);
// Center text: x = (screen_width - text_width) / 2
```

#### 7-Segment Display Functions

##### `text_render_7segment_digit()`
```c
void text_render_7segment_digit(TextRenderer* tr, char digit, int x, int y, 
                                SDL_Color color, int scale);
```
Render single digit in 7-segment display style.

**Parameters**:
- `digit`: Character digit '0'-'9' or space for blank
- `scale`: Scale factor (1 = normal size, 2 = double size, etc.)

**Example**:
```c
SDL_Color red = {255, 0, 0, 255};
text_render_7segment_digit(&text_renderer, '5', 100, 50, red, 2);
```

##### `text_render_7segment_string()`
```c
void text_render_7segment_string(TextRenderer* tr, const char* numbers, 
                                 int x, int y, SDL_Color color, int scale);
```
Render numeric string in 7-segment display style.

**Parameters**:
- `numbers`: Numeric string (digits 0-9, spaces, decimal points)
- `scale`: Scale factor for segment size

**Example**:
```c
// Render timer display
text_render_7segment_string(&text_renderer, "12:34", 200, 100, red, 3);
```

##### `text_get_7segment_dimensions()`
```c
void text_get_7segment_dimensions(const char* text, int scale, int* width, int* height);
```
Get 7-segment display dimensions for layout.

#### Utility Functions

##### `text_renderer_set_default_color()`
```c
void text_renderer_set_default_color(TextRenderer* tr, SDL_Color color);
```
Set default text color for [`text_render_string_default()`](#text_render_string_default).

##### `text_render_char()`
```c
void text_render_char(TextRenderer* tr, char c, int x, int y, SDL_Color color);
```
Render single character (advanced usage).

##### `text_renderer_is_ready()`
```c
bool text_renderer_is_ready(const TextRenderer* tr);
```
Check if text renderer is properly initialized.

---

## Font Data

### `font_data.h`

5x7 bitmap font data and character mapping.

#### Constants

```c
#define GLYPH_COUNT 60          // Total glyphs available
#define FONT_WIDTH 5            // Character width in pixels
#define FONT_HEIGHT 7           // Character height in pixels  
#define CHAR_SPACING 6          // 5 pixels + 1 pixel spacing
```

#### Functions

##### `font_get_char_index()`
```c
int font_get_char_index(char c);
```
Maps ASCII characters to font glyph indices.

**Returns**: Glyph index (0-59), 0 for unknown characters (renders as space)

**Supported Characters**: 
- Digits: `0-9`
- Letters: `A-Z` (uppercase)
- Punctuation: `. , ! ? : ;`
- Special: Arrow symbols, mathematical operators

##### `font_get_glyph_pattern()`
```c
const uint8_t* font_get_glyph_pattern(int glyph_index);
```
Get 7-row bitmap pattern for specified glyph.

**Returns**: Pointer to 7-byte glyph pattern, `NULL` for invalid index

##### `font_get_text_dimensions()`
```c
void font_get_text_dimensions(const char* text, int* width, int* height);
```
Calculate text dimensions. Identical to [`text_get_dimensions()`](#text_get_dimensions).

##### `font_validate_data()`
```c
void font_validate_data(void);
```
Validate font data integrity (debug builds only). Called automatically by [`shared_components_init()`](#shared_components_init).

---

## UI Framework

### UI Buttons

#### `ui_button.h`

Interactive button system with state management and callbacks.

#### Structures

##### `UIButton`
```c
typedef struct {
    SDL_FRect rect;               // Button bounds
    char text[32];                // Button text
    UIButtonState state;          // Current button state
    UIButtonCallback on_click;    // Click callback function
    void* userdata;               // User data for callback
    
    // Visual styling
    SDL_Color bg_color_normal;    // Normal background color
    SDL_Color bg_color_hover;     // Hover background color
    SDL_Color bg_color_pressed;   // Pressed background color
    SDL_Color bg_color_disabled;  // Disabled background color
    SDL_Color text_color;         // Text color
    SDL_Color border_color;       // Border color
    
    // Internal state
    bool visible;                 // Whether button is visible
    int id;                       // Optional button ID
} UIButton;
```

##### `UIButtonArray`
```c
typedef struct {
    UIButton* buttons;      // Array of buttons
    int count;              // Number of buttons
    int capacity;           // Array capacity
    int hovered_button;     // Index of currently hovered button (-1 if none)
    int pressed_button;     // Index of currently pressed button (-1 if none)
} UIButtonArray;
```

##### `UIButtonState`
```c
typedef enum {
    UI_BUTTON_NORMAL = 0x00,
    UI_BUTTON_HOVERED = 0x01,
    UI_BUTTON_PRESSED = 0x02,
    UI_BUTTON_DISABLED = 0x04,
    UI_BUTTON_SELECTED = 0x08
} UIButtonState;
```

##### `UIButtonCallback`
```c
typedef void (*UIButtonCallback)(void* userdata);
```
Button callback function type called when button is clicked.

#### Single Button Functions

##### `ui_button_init()`
```c
void ui_button_init(UIButton* button, int x, int y, int w, int h, const char* text);
```
Initialize button with default settings.

**Example**:
```c
UIButton save_button;
ui_button_init(&save_button, 10, 10, 80, 30, "Save");
```

##### `ui_button_set_callback()`
```c
void ui_button_set_callback(UIButton* button, UIButtonCallback callback, void* userdata);
```
Set button callback function.

**Example**:
```c
void save_clicked(void* userdata) {
    printf("Save button clicked!\n");
}

ui_button_set_callback(&save_button, save_clicked, NULL);
```

##### `ui_button_set_colors()`
```c
void ui_button_set_colors(UIButton* button, SDL_Color normal, SDL_Color hover, 
                          SDL_Color pressed, SDL_Color disabled);
```
Set button colors for different states.

##### `ui_button_handle_input()`
```c
bool ui_button_handle_input(UIButton* button, int mouse_x, int mouse_y, bool clicked);
```
Handle mouse input for button. Call once per frame.

**Returns**: `true` if button was clicked and callback executed

**Example**:
```c
// In event loop
SDL_Event event;
while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        int mx, my;
        SDL_GetMouseState(&mx, &my);
        ui_button_handle_input(&save_button, mx, my, true);
    }
}
```

##### `ui_button_render()`
```c
void ui_button_render(UIButton* button, SDL_Renderer* renderer, TextRenderer* text_renderer);
```
Render button. Pass `NULL` for `text_renderer` for simple rendering without text.

#### Button Array Functions

##### `ui_button_array_init()`
```c
bool ui_button_array_init(UIButtonArray* array, int initial_capacity);
```
Initialize button array with initial capacity.

**Returns**: `true` if successful, `false` on memory allocation error

##### `ui_button_array_add()`
```c
int ui_button_array_add(UIButtonArray* array, const UIButton* button);
```
Add button to array (button is copied).

**Returns**: Index of added button, -1 on error

##### `ui_button_array_handle_input()`
```c
int ui_button_array_handle_input(UIButtonArray* array, int mouse_x, int mouse_y, bool clicked);
```
Handle mouse input for all buttons in array.

**Returns**: Index of clicked button, -1 if none clicked

**Example**:
```c
int clicked_button = ui_button_array_handle_input(&buttons, mouse_x, mouse_y, clicked);
if (clicked_button >= 0) {
    printf("Button %d was clicked!\n", clicked_button);
}
```

### UI Primitives

#### `ui_primitives.h`

Basic rendering utilities and coordinate functions.

#### Rendering Functions

##### `ui_render_rect()`
```c
void ui_render_rect(SDL_Renderer* renderer, int x, int y, int w, int h, SDL_Color color);
```
Render filled rectangle with specified color.

##### `ui_render_rect_outline()`
```c
void ui_render_rect_outline(SDL_Renderer* renderer, int x, int y, int w, int h, SDL_Color color);
```
Render rectangle outline with specified color.

##### `ui_render_rect_f()`
```c
void ui_render_rect_f(SDL_Renderer* renderer, const SDL_FRect* rect, SDL_Color color);
```
Render filled rectangle using floating point coordinates.

#### Coordinate Utilities

##### `ui_point_in_rect()`
```c
bool ui_point_in_rect(int x, int y, const SDL_FRect* rect);
```
Check if point is inside rectangle.

**Example**:
```c
SDL_FRect button_rect = {10, 10, 80, 30};
if (ui_point_in_rect(mouse_x, mouse_y, &button_rect)) {
    // Mouse is over button
}
```

##### `ui_make_rect()`
```c
SDL_FRect ui_make_rect(int x, int y, int w, int h);
```
Create SDL_FRect structure from integer coordinates.

##### `ui_clamp_int()`
```c
int ui_clamp_int(int value, int min, int max);
```
Clamp integer value to specified range.

**Example**:
```c
int clamped_volume = ui_clamp_int(volume, 0, 100);
```

---

## Palette Manager

### `palette_manager.h`

16-color RGBA palette management with file I/O and modification tracking.

#### Structures

##### `RGBA`
```c
typedef struct {
    uint8_t r, g, b, a;
} RGBA;
```
Standard 8-bit per channel color representation.

##### `PaletteManager`
```c
typedef struct {
    RGBA colors[PALETTE_COLOR_COUNT];         // 16 colors
    bool modified;                            // Modification flag
    char current_file[PALETTE_FILENAME_MAX];  // Current palette file path
    bool file_loaded;                         // Whether a file has been loaded
} PaletteManager;
```

#### Constants

```c
#define PALETTE_COLOR_COUNT 16
#define PALETTE_FILENAME_MAX 256
```

#### Core Functions

##### `palette_manager_init()`
```c
bool palette_manager_init(PaletteManager* pm);
```
Initialize palette manager with default 16-color palette.

**Example**:
```c
PaletteManager palette;
if (!palette_manager_init(&palette)) {
    fprintf(stderr, "Failed to initialize palette\n");
}
```

##### `palette_get_color()`
```c
RGBA palette_get_color(const PaletteManager* pm, int index);
```
Get color at specified index (0-15).

**Returns**: RGBA color, black if invalid index

##### `palette_set_color()`
```c
bool palette_set_color(PaletteManager* pm, int index, RGBA color);
```
Set color at specified index. Automatically marks palette as modified.

**Returns**: `true` if successful, `false` if invalid index

##### `palette_get_sdl_color()`
```c
SDL_Color palette_get_sdl_color(const PaletteManager* pm, int index);
```
Get SDL_Color for rendering. Convenience function.

**Example**:
```c
SDL_Color bg_color = palette_get_sdl_color(&palette, 0);  // Background color
SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
```

#### File I/O Functions

##### `palette_manager_load()`
```c
bool palette_manager_load(PaletteManager* pm, const char* filepath);
```
Load palette from file. Supports standard `.pal` format (64 bytes RGBA).

**Returns**: `true` if successful, `false` on error

**Example**:
```c
if (!palette_manager_load(&palette, "mypalette.pal")) {
    printf("Failed to load palette file\n");
}
```

##### `palette_manager_save()`
```c
bool palette_manager_save(PaletteManager* pm, const char* filepath);
```
Save palette to file. Pass `NULL` for filepath to use current file.

**Returns**: `true` if successful, `false` on error

##### `palette_manager_is_modified()`
```c
bool palette_manager_is_modified(const PaletteManager* pm);
```
Check if palette has been modified since last save/load.

#### Utility Functions

##### `palette_make_color()`
```c
RGBA palette_make_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
```
Create RGBA color from individual components.

##### `palette_manager_copy()`
```c
void palette_manager_copy(PaletteManager* dest, const PaletteManager* src);
```
Copy all colors from source palette to destination.

---

## SDL Framework

### `sdl_context.h`

SDL3 initialization and context management framework.

#### Structures

##### `SDLContext`
```c
typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    int width;
    int height;
    char title[128];
    bool initialized;
    bool vsync_enabled;
} SDLContext;
```

##### `SDLContextConfig`
```c
typedef struct {
    const char* title;
    int width;
    int height;
    bool resizable;
    bool vsync;
    bool fullscreen;
} SDLContextConfig;
```

#### Core Functions

##### `sdl_init_context_simple()`
```c
bool sdl_init_context_simple(SDLContext* ctx, const char* title, int width, int height);
```
Initialize SDL context with default settings.

**Parameters**:
- `ctx`: SDL context to initialize
- `title`: Window title
- `width`, `height`: Window dimensions

**Returns**: `true` if successful, `false` on error

**Example**:
```c
SDLContext sdl_ctx;
if (!sdl_init_context_simple(&sdl_ctx, "My Game", 800, 600)) {
    fprintf(stderr, "SDL initialization failed: %s\n", sdl_get_error());
    exit(1);
}
```

##### `sdl_init_context()`
```c
bool sdl_init_context(SDLContext* ctx, const SDLContextConfig* config);
```
Initialize SDL context with full configuration options.

##### `sdl_cleanup_context()`
```c
void sdl_cleanup_context(SDLContext* ctx);
```
Cleanup SDL context and free all resources.

##### `sdl_get_renderer()`
```c
SDL_Renderer* sdl_get_renderer(const SDLContext* ctx);
```
Get SDL renderer from context.

#### Rendering Utilities

##### `sdl_clear_screen()`
```c
void sdl_clear_screen(SDLContext* ctx, SDL_Color color);
```
Clear screen with specified color.

**Example**:
```c
SDL_Color black = {0, 0, 0, 255};
sdl_clear_screen(&sdl_ctx, black);
```

##### `sdl_present()`
```c
void sdl_present(SDLContext* ctx);
```
Present rendered frame to screen.

##### `sdl_set_logical_presentation()`
```c
bool sdl_set_logical_presentation(SDLContext* ctx, int width, int height);
```
Set logical presentation for consistent UI scaling across different screen sizes.

---

## Utilities

### Double-Click Detection

#### `double_click.h`

Configurable double-click detection utility.

#### Structures

##### `DoubleClickDetector`
```c
typedef struct {
    Uint64 last_click_time;   // Time of last click in milliseconds  
    int last_clicked_target;  // ID of last clicked target
    Uint32 threshold_ms;      // Double-click threshold in milliseconds
} DoubleClickDetector;
```

#### Functions

##### `double_click_init()`
```c
void double_click_init(DoubleClickDetector* detector, Uint32 threshold_ms);
```
Initialize double-click detector. Pass 0 for threshold_ms to use default (300ms).

##### `double_click_check()`
```c
bool double_click_check(DoubleClickDetector* detector, int target_id);
```
Check if current click is a double-click. Call when a click occurs.

**Parameters**:
- `detector`: Double-click detector
- `target_id`: ID of clicked target (e.g., button ID, palette swatch index)

**Returns**: `true` if this click is a double-click

**Example**:
```c
DoubleClickDetector double_click;
double_click_init(&double_click, DOUBLE_CLICK_THRESHOLD_NORMAL);

// In event handling
if (mouse_clicked) {
    if (double_click_check(&double_click, button_id)) {
        printf("Double-click detected on button %d!\n", button_id);
    }
}
```

#### Constants

```c
#define DOUBLE_CLICK_THRESHOLD_FAST 200    // Fast double-click (200ms)
#define DOUBLE_CLICK_THRESHOLD_NORMAL 300  // Normal double-click (300ms)  
#define DOUBLE_CLICK_THRESHOLD_SLOW 500    // Slow double-click (500ms)
```

### File Utils

#### `file_utils.h`

Common file operations and path manipulation utilities.

#### Functions

##### `file_exists()`
```c
bool file_exists(const char* filepath);
```
Check if file exists and is readable.

##### `file_get_size()`
```c
long file_get_size(const char* filepath);
```
Get file size in bytes. Returns -1 on error.

##### `file_get_filename()`
```c
bool file_get_filename(const char* filepath, char* filename, size_t filename_size);
```
Extract filename from full path.

**Example**:
```c
char filename[256];
if (file_get_filename("/path/to/myfile.txt", filename, sizeof(filename))) {
    printf("Filename: %s\n", filename);  // Prints: myfile.txt
}
```

##### `file_get_extension()`
```c
bool file_get_extension(const char* filepath, char* extension, size_t extension_size);
```
Extract file extension from path (without dot).

##### `file_join_path()`
```c
bool file_join_path(const char* directory, const char* filename, char* output, size_t output_size);
```
Join directory and filename into full path with proper path separators.

##### `file_write_atomic()`
```c
bool file_write_atomic(const char* filepath, const void* data, size_t size);
```
Safely write data to file using atomic operation (writes to temporary file first, then renames).

**Example**:
```c
char data[] = "Important data";
if (!file_write_atomic("important.dat", data, strlen(data))) {
    fprintf(stderr, "Failed to save data\n");
}
```

---

*This API reference covers all public functions and structures in the Shared Components Library v1.0.0*

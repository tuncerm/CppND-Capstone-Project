# Migration Guide - Shared Components Library

Guide for migrating existing palette-maker and tile-maker applications to use the shared components library.

## Table of Contents

- [Overview](#overview)
- [Breaking Changes](#breaking-changes)
- [Migration Steps](#migration-steps)
- [Component Migration](#component-migration)
- [Code Examples](#code-examples)
- [Testing Migration](#testing-migration)
- [Common Issues](#common-issues)

---

## Overview

The shared components library extracts common functionality from palette-maker and tile-maker into reusable components. This migration guide helps update existing code to use the new unified APIs.

### Migration Benefits

- **Reduced Code Duplication**: Eliminate duplicate UI and rendering code
- **Improved Maintainability**: Centralized bug fixes and improvements
- **Enhanced Features**: Access to improved text rendering and 7-segment displays
- **Consistent APIs**: Unified interfaces across applications
- **Better Testing**: Shared components have comprehensive test coverage

### Migration Scope

This guide covers migrating:
- Text rendering systems
- UI button implementations
- Palette management code
- SDL initialization patterns
- Double-click detection
- File utility functions

---

## Breaking Changes

### API Changes

| Old Function/Pattern | New Function | Breaking Change |
|---------------------|--------------|----------------|
| Direct glyph access | [`text_render_string()`](API.md#text_render_string) | ✅ Simplified API |
| Manual SDL setup | [`sdl_init_context_simple()`](API.md#sdl_init_context_simple) | ✅ Structured initialization |
| Global palette arrays | [`PaletteManager`](API.md#palettemanager) struct | ✅ Object-oriented approach |
| Coordinate-based buttons | [`UIButton`](API.md#uibutton) struct | ✅ State management |
| Manual double-click timing | [`DoubleClickDetector`](API.md#doubleclickdetector) | ✅ Configurable thresholds |

### Header Changes

```c
// Old includes
#include "ui.h"           // Application-specific UI
#include "palette.h"      // Application-specific palette

// New includes  
#include <shared_components.h>  // Single header for all components
```

### Initialization Changes

```c
// Old: Manual initialization
SDL_Init(SDL_INIT_VIDEO);
window = SDL_CreateWindow(...);
renderer = SDL_CreateRenderer(...);

// New: Structured initialization
SDLContext ctx;
sdl_init_context_simple(&ctx, "App", 800, 600);
```

---

## Migration Steps

### Step 1: Add Shared Components Dependency

#### Update CMakeLists.txt

```cmake
# Add to your CMakeLists.txt
add_subdirectory(shared)
target_link_libraries(your_app PRIVATE shared_components SDL3::SDL3)
target_include_directories(your_app PRIVATE shared)
```

#### Update Build Scripts

```bash
# Update build.bat (Windows)
cmake --build . --config Release

# Update build.sh (Unix)
make -j$(nproc)
```

### Step 2: Update Includes

Replace application-specific includes with shared components:

```c
// Remove old includes
// #include "ui.h"
// #include "palette.h"  
// #include "font.h"

// Add new include
#include <shared_components.h>
```

### Step 3: Initialize Shared Components

Add initialization call at application startup:

```c
int main(int argc, char* argv[]) {
    // Add this as first step
    if (!shared_components_init()) {
        fprintf(stderr, "Failed to initialize shared components\n");
        return 1;
    }
    
    // ... rest of initialization
    
    // Add cleanup before exit
    shared_components_cleanup();
    return 0;
}
```

### Step 4: Migrate Component by Component

Follow the component-specific migration instructions below.

---

## Component Migration

### Text Rendering Migration

#### From palette-maker Text System

**Old Code** (palette-maker/ui.c):
```c
// Old direct glyph rendering
static const uint8_t font_5x7[60][7] = { /* glyph data */ };

int get_char_index(char c) {
    // Character mapping logic
}

void ui_render_text(SDL_Renderer* renderer, const char* text, 
                   int x, int y, SDL_Color color) {
    // Manual glyph rendering
    for (int i = 0; text[i] && i < 32; i++) {
        int char_index = get_char_index(text[i]);
        const uint8_t* glyph = font_5x7[char_index];
        
        for (int row = 0; row < 7; row++) {
            for (int col = 0; col < 5; col++) {
                if (glyph[row] & (1 << (4 - col))) {
                    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
                    SDL_RenderPoint(renderer, x + i * 6 + col, y + row);
                }
            }
        }
    }
}
```

**New Code**:
```c
// Initialize text renderer once
TextRenderer text_renderer;
text_renderer_init(&text_renderer, renderer);

// Render text with simple call
SDL_Color white = {255, 255, 255, 255};
text_render_string(&text_renderer, "Hello World", 10, 10, white);

// Set default color for frequent use
text_renderer_set_default_color(&text_renderer, white);
text_render_string_default(&text_renderer, "Default color text", 10, 30);
```

#### From tile-maker Placeholder System

**Old Code** (tile-maker/ui.c):
```c
// Old placeholder text rendering
void render_text_placeholder(SDL_Renderer* renderer, const char* text, 
                            int x, int y, int w, int h) {
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderRect(renderer, &rect);
    
    // No actual text rendering
}
```

**New Code**:
```c
// Full text rendering capability
TextRenderer text_renderer;
text_renderer_init(&text_renderer, renderer);

SDL_Color gray = {128, 128, 128, 255};
text_render_string(&text_renderer, "Actual Text!", x, y, gray);

// Get text dimensions for proper layout
int text_width, text_height;
text_get_dimensions("Actual Text!", &text_width, &text_height);
```

#### 7-Segment Display Addition

**New Feature** (not available in original code):
```c
// Digital clock display
SDL_Color red = {255, 0, 0, 255};
text_render_7segment_string(&text_renderer, "12:34", 100, 50, red, 2);

// Large single digit
text_render_7segment_digit(&text_renderer, '8', 200, 100, red, 3);
```

### UI Button Migration

#### From palette-maker Coordinate System

**Old Code** (palette-maker/ui.c):
```c
// Old coordinate-based buttons
#define IS_BUTTON_CLICKED(mx, my, x, y, w, h) \
    ((mx) >= (x) && (mx) < (x) + (w) && (my) >= (y) && (my) < (y) + (h))

void handle_button_clicks(int mouse_x, int mouse_y) {
    if (IS_BUTTON_CLICKED(mouse_x, mouse_y, 100, 50, 80, 30)) {
        // Save button clicked
        save_palette();
    }
    if (IS_BUTTON_CLICKED(mouse_x, mouse_y, 200, 50, 80, 30)) {
        // Load button clicked  
        load_palette();
    }
}

void render_buttons(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_Rect save_rect = {100, 50, 80, 30};
    SDL_RenderFillRect(renderer, &save_rect);
    
    SDL_Rect load_rect = {200, 50, 80, 30};
    SDL_RenderFillRect(renderer, &load_rect);
}
```

**New Code**:
```c
// Structured button system
UIButtonArray buttons;
ui_button_array_init(&buttons, 10);

// Create save button with callback
UIButton save_button;
ui_button_init(&save_button, 100, 50, 80, 30, "Save");
ui_button_set_callback(&save_button, save_palette_callback, palette_data);
ui_button_array_add(&buttons, &save_button);

// Create load button
UIButton load_button;  
ui_button_init(&load_button, 200, 50, 80, 30, "Load");
ui_button_set_callback(&load_button, load_palette_callback, palette_data);
ui_button_array_add(&buttons, &load_button);

// Handle input (in event loop)
int clicked = ui_button_array_handle_input(&buttons, mouse_x, mouse_y, clicked);

// Render all buttons
ui_button_array_render(&buttons, renderer, &text_renderer);
```

#### From tile-maker Structured System

**Old Code** (tile-maker/ui.c):
```c
// Old structured approach
typedef struct {
    SDL_Rect rect;
    char text[32];
    bool pressed;
    bool hovered;
} UIButton;

UIButton buttons[10];
int button_count = 0;

void update_button_states(int mouse_x, int mouse_y) {
    for (int i = 0; i < button_count; i++) {
        bool over = (mouse_x >= buttons[i].rect.x && 
                    mouse_x < buttons[i].rect.x + buttons[i].rect.w &&
                    mouse_y >= buttons[i].rect.y &&
                    mouse_y < buttons[i].rect.y + buttons[i].rect.h);
        buttons[i].hovered = over;
    }
}
```

**New Code**:
```c
// Enhanced structured system with callbacks
UIButtonArray buttons;
ui_button_array_init(&buttons, 10);

// Buttons automatically handle hover/press states
// and execute callbacks when clicked
UIButton my_button;
ui_button_init(&my_button, 10, 10, 100, 40, "Click Me");
ui_button_set_callback(&my_button, my_callback, user_data);

// Set custom colors for different states
SDL_Color normal = {100, 100, 100, 255};
SDL_Color hover = {120, 120, 120, 255};  
SDL_Color pressed = {80, 80, 80, 255};
SDL_Color disabled = {60, 60, 60, 255};
ui_button_set_colors(&my_button, normal, hover, pressed, disabled);

ui_button_array_add(&buttons, &my_button);
```

### Palette Management Migration

#### From palette-maker Struct System

**Old Code** (palette-maker/palette.c):
```c
// Old palette structure
typedef struct {
    PaletteColor colors[16];
} Palette;

typedef struct {
    unsigned char r, g, b, a;
} PaletteColor;

Palette current_palette;

bool save_palette(const Palette* palette, const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (!file) return false;
    
    fwrite(palette->colors, sizeof(PaletteColor), 16, file);
    fclose(file);
    return true;
}
```

**New Code**:
```c
// Unified palette manager
PaletteManager palette;
palette_manager_init(&palette);

// Same functionality with enhanced features
if (!palette_manager_load(&palette, "palette.pal")) {
    printf("Failed to load palette\n");
}

// Modification tracking built-in
RGBA new_color = palette_make_color(255, 128, 64, 255);
palette_set_color(&palette, 5, new_color);

// Automatic modification detection
if (palette_manager_is_modified(&palette)) {
    palette_manager_save(&palette, NULL); // Save to current file
}

// SDL integration helpers
SDL_Color render_color = palette_get_sdl_color(&palette, 0);
```

#### From tile-maker Global Array System

**Old Code** (tile-maker/palette_io.c):
```c
// Old global array approach
RGBA gPalette[16];

void load_palette(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file) {
        fread(gPalette, sizeof(RGBA), 16, file);
        fclose(file);
    }
}

void save_palette(const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (file) {
        fwrite(gPalette, sizeof(RGBA), 16, file);
        fclose(file);
    }
}
```

**New Code**:
```c
// Object-oriented approach with better error handling
PaletteManager palette;
palette_manager_init(&palette);

// Enhanced file operations
if (!palette_manager_load(&palette, filename)) {
    fprintf(stderr, "Failed to load palette: %s\n", filename);
    // Fallback to defaults
    palette_manager_reset_to_default(&palette);
}

// Safe file operations
if (!palette_manager_save(&palette, filename)) {
    fprintf(stderr, "Failed to save palette: %s\n", filename);
}

// Access colors same way but with bounds checking
RGBA color = palette_get_color(&palette, index); // Returns black if invalid index
```

### SDL Initialization Migration

#### From Manual SDL Setup

**Old Code** (both applications):
```c
// palette-maker/main.c & tile-maker/main.c
if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
    return 1;
}

SDL_Window* window = SDL_CreateWindow("Application",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    800, 600, SDL_WINDOW_SHOWN);
if (!window) {
    fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
}

SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 
    SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
if (!renderer) {
    fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
}

SDL_SetRenderLogicalPresentation(renderer, 800, 600,
    SDL_LOGICAL_PRESENTATION_LETTERBOX, SDL_SCALEMODE_LINEAR);
```

**New Code**:
```c
// Simplified initialization
SDLContext sdl_ctx;
if (!sdl_init_context_simple(&sdl_ctx, "My Application", 800, 600)) {
    fprintf(stderr, "SDL initialization failed: %s\n", sdl_get_error());
    return 1;
}

// Access components
SDL_Renderer* renderer = sdl_get_renderer(&sdl_ctx);
SDL_Window* window = sdl_get_window(&sdl_ctx);

// Optional: Set logical presentation
sdl_set_logical_presentation(&sdl_ctx, 800, 600);

// Cleanup
sdl_cleanup_context(&sdl_ctx);
```

### Double-Click Detection Migration

#### From Timing-Based Detection

**Old Code** (palette-maker: 300ms, tile-maker: 500ms):
```c
// palette-maker/ui.c
#define DOUBLE_CLICK_TIME 300
static Uint64 last_click_time = 0;
static int last_clicked_swatch = -1;

bool is_double_click(int swatch_index) {
    Uint64 current_time = SDL_GetTicks();
    bool double_click = (current_time - last_click_time <= DOUBLE_CLICK_TIME && 
                        last_clicked_swatch == swatch_index);
    
    last_click_time = current_time;
    last_clicked_swatch = swatch_index;
    
    return double_click;
}

// tile-maker/ui.c  
static Uint64 last_click_time = 0;
static int last_clicked_tile = -1;

bool check_double_click(int tile_index) {
    Uint64 current_time = SDL_GetTicks();
    bool is_double = (current_time - last_click_time < 500 && 
                     last_clicked_tile == tile_index);
    
    last_click_time = current_time;
    last_clicked_tile = tile_index;
    
    return is_double;
}
```

**New Code**:
```c
// Unified configurable system
DoubleClickDetector double_click;

// Use palette-maker's timing (300ms)
double_click_init(&double_click, DOUBLE_CLICK_THRESHOLD_NORMAL);

// Or use tile-maker's timing (500ms)  
double_click_init(&double_click, DOUBLE_CLICK_THRESHOLD_SLOW);

// Or custom timing
double_click_init(&double_click, 400); // 400ms threshold

// Usage (same for any target type)
if (double_click_check(&double_click, target_id)) {
    printf("Double-click detected on target %d\n", target_id);
    handle_double_click(target_id);
}
```

---

## Code Examples

### Complete Migration Example

#### Before (palette-maker style):
```c
// Old palette-maker main loop
int main() {
    // Manual SDL setup
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Palette Maker", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    // Manual palette setup
    Palette palette;
    initialize_default_palette(&palette);
    
    // Event loop
    bool running = true;
    SDL_Event event;
    
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                
                // Manual button detection
                if (IS_BUTTON_CLICKED(mx, my, 100, 50, 80, 30)) {
                    save_palette(&palette, "palette.pal");
                }
                
                // Manual double-click detection
                int swatch = get_clicked_swatch(mx, my);
                if (swatch >= 0 && is_double_click(swatch)) {
                    edit_color(&palette, swatch);
                }
            }
        }
        
        // Manual rendering
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        render_palette(&palette, renderer);
        render_buttons(renderer);
        ui_render_text(renderer, "Palette Maker", 10, 10, 
                      (SDL_Color){255, 255, 255, 255});
        
        SDL_RenderPresent(renderer);
    }
    
    // Manual cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
```

#### After (shared components):
```c
// New shared components approach
int main() {
    // Initialize shared components
    if (!shared_components_init()) {
        return 1;
    }
    
    // Simplified SDL setup
    SDLContext sdl_ctx;
    if (!sdl_init_context_simple(&sdl_ctx, "Palette Maker", 800, 600)) {
        shared_components_cleanup();
        return 1;
    }
    
    // Initialize components
    TextRenderer text_renderer;
    text_renderer_init(&text_renderer, sdl_get_renderer(&sdl_ctx));
    
    PaletteManager palette;
    palette_manager_init(&palette);
    
    UIButtonArray buttons;
    ui_button_array_init(&buttons, 5);
    
    DoubleClickDetector double_click;
    double_click_init(&double_click, DOUBLE_CLICK_THRESHOLD_NORMAL);
    
    // Create save button
    UIButton save_button;
    ui_button_init(&save_button, 100, 50, 80, 30, "Save");
    ui_button_set_callback(&save_button, save_palette_callback, &palette);
    ui_button_array_add(&buttons, &save_button);
    
    // Event loop
    bool running = true;
    SDL_Event event;
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color black = {0, 0, 0, 255};
    
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                
                // Automatic button handling
                ui_button_array_handle_input(&buttons, mx, my, true);
                
                // Simplified double-click detection
                int swatch = get_clicked_swatch(mx, my);
                if (swatch >= 0 && double_click_check(&double_click, swatch)) {
                    edit_color_dialog(&palette, swatch);
                }
            }
        }
        
        // Simplified rendering
        sdl_clear_screen(&sdl_ctx, black);
        
        render_palette(&palette, sdl_get_renderer(&sdl_ctx));
        ui_button_array_render(&buttons, sdl_get_renderer(&sdl_ctx), &text_renderer);
        text_render_string(&text_renderer, "Palette Maker", 10, 10, white);
        
        sdl_present(&sdl_ctx);
    }
    
    // Automatic cleanup
    ui_button_array_cleanup(&buttons);
    text_renderer_cleanup(&text_renderer);
    sdl_cleanup_context(&sdl_ctx);
    shared_components_cleanup();
    
    return 0;
}

// Callback function
void save_palette_callback(void* userdata) {
    PaletteManager* palette = (PaletteManager*)userdata;
    if (!palette_manager_save(palette, "palette.pal")) {
        printf("Failed to save palette\n");
    }
}
```

---

## Testing Migration

### Validation Checklist

- [ ] Application compiles without errors
- [ ] All text renders correctly (check character mapping)
- [ ] Buttons respond to mouse input  
- [ ] Button callbacks execute properly
- [ ] Palette loading/saving works
- [ ] SDL initialization succeeds
- [ ] Double-click detection works as expected
- [ ] Application cleanup completes without leaks
- [ ] Performance is equivalent or better

### Testing Procedures

#### 1. Text Rendering Test
```c
// Test all character types
text_render_string(&text_renderer, "0123456789", 10, 10, white);
text_render_string(&text_renderer, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 10, 30, white);
text_render_string(&text_renderer, "!@#$%^&*()[]{},.?", 10, 50, white);

// Test 7-segment displays
text_render_7segment_string(&text_renderer, "12:34:56", 10, 100, red, 2);
```

#### 2. Button Interaction Test
```c
// Create test buttons
for (int i = 0; i < 5; i++) {
    UIButton btn;
    char text[32];
    snprintf(text, sizeof(text), "Button %d", i);
    ui_button_init(&btn, 10 + i * 90, 10, 80, 30, text);
    ui_button_set_callback(&btn, test_button_callback, &i);
    ui_button_array_add(&buttons, &btn);
}

void test_button_callback(void* userdata) {
    int* button_id = (int*)userdata;
    printf("Button %d clicked!\n", *button_id);
}
```

#### 3. Palette Compatibility Test
```c
// Test loading old palette files
PaletteManager new_palette;
palette_manager_init(&new_palette);

if (palette_manager_load(&new_palette, "old_palette.pal")) {
    printf("Successfully loaded old palette\n");
    
    // Verify colors match expected values
    for (int i = 0; i < 16; i++) {
        RGBA color = palette_get_color(&new_palette, i);
        printf("Color %d: R=%d G=%d B=%d A=%d\n", i, color.r, color.g, color.b, color.a);
    }
} else {
    printf("Failed to load old palette - check format compatibility\n");
}
```

### Performance Comparison

```c
// Benchmark text rendering
Uint64 start_time = SDL_GetPerformanceCounter();

for (int i = 0; i < 1000; i++) {
    text_render_string(&text_renderer, "Performance Test", 10, 10, white);
}

Uint64 end_time = SDL_GetPerformanceCounter();
double ms = (double)(end_time - start_time) / SDL_GetPerformanceFrequency() * 1000.0;
printf("1000 text renders took %.2f ms\n", ms);
```

---

## Common Issues

### Issue 1: Text Not Rendering

**Symptoms**: Text appears as blank spaces or doesn't render at all

**Causes**:
- Text renderer not initialized
- Invalid SDL renderer passed
- Color has zero alpha

**Solutions**:
```c
// Verify text renderer initialization
if (!text_renderer_is_ready(&text_renderer)) {
    text_renderer_cleanup(&text_renderer);
    text_renderer_init(&text_renderer, sdl_get_renderer(&sdl_ctx));
}

// Check color alpha
SDL_Color color = {255, 255, 255, 255}; // Ensure alpha = 255
text_render_string(&text_renderer, "Test", 10, 10, color);
```

### Issue 2: Buttons Not Responding

**Symptoms**: Button clicks don't trigger callbacks

**Causes**:
- Callback not set
- Incorrect mouse coordinates
- Button array not initialized

**Solutions**:
```c
// Verify button setup
UIButton btn;
ui_button_init(&btn, 10, 10, 100, 30, "Test");
ui_button_set_callback(&btn, test_callback, NULL); // Don't forget callback!

// Debug mouse coordinates
printf("Mouse: %d, %d | Button: %.0f, %.0f, %.0f, %.0f\n", 
       mouse_x, mouse_y, btn.rect.x, btn.rect.y, btn.rect.w, btn.rect.h);
```

### Issue 3: Palette Files Not Loading

**Symptoms**: Palette files fail to load or have incorrect colors

**Causes**:
- File format differences
- Endianness issues
- File path problems

**Solutions**:
```c
// Validate file before loading
if (!file_exists("palette.pal")) {
    printf("Palette file doesn't exist\n");
    return;
}

if (!palette_manager_validate_file("palette.pal")) {
    printf("Invalid palette file format\n");
    return;
}

// Check file size
long size = file_get_size("palette.pal");
if (size != 64) { // 16 colors * 4 bytes (RGBA)
    printf("Unexpected palette file size: %ld bytes (expected 64)\n", size);
}
```

### Issue 4: Memory Leaks

**Symptoms**: Application memory usage increases over time

**Causes**:
- Missing cleanup calls
- Button array not freed
- SDL context not cleaned up

**Solutions**:
```c
// Always cleanup in reverse order of initialization
void cleanup_application(void) {
    ui_button_array_cleanup(&buttons);        // Free button array
    text_renderer_cleanup(&text_renderer);    // Cleanup text renderer  
    sdl_cleanup_context(&sdl_ctx);           // Cleanup SDL
    shared_components_cleanup();              // Cleanup shared components
}

// Use atexit() for automatic cleanup
atexit(cleanup_application);
```

### Issue 5: Performance Degradation

**Symptoms**: Application runs slower than before migration

**Causes**:
- Excessive text rendering calls
- Inefficient button handling
- Missing vsync

**Solutions**:
```c
// Cache text that doesn't change frequently
static bool score_text_cached = false;
static char score_text[32];

if (!score_text_cached || score_changed) {
    snprintf(score_text, sizeof(score_text), "Score: %d", current_score);
    score_text_cached = true;
    score_changed = false;
}
text_render_string(&text_renderer, score_text, 10, 10, white);

// Enable vsync in SDL context
SDLContextConfig config = {
    .title = "My App",
    .width = 800,
    .height = 600,
    .vsync = true  // Important for smooth rendering
};
sdl_init_context(&sdl_ctx, &config);
```

---

## Migration Timeline

### Phase 1: Preparation (1-2 days)
- [ ] Add shared components to build system
- [ ] Update includes and headers
- [ ] Initialize shared components in main()

### Phase 2: Core Components (2-3 days)  
- [ ] Migrate text rendering system
- [ ] Update SDL initialization
- [ ] Convert palette management

### Phase 3: UI Components (2-3 days)
- [ ] Convert button systems
- [ ] Implement double-click detection
- [ ] Update event handling

### Phase 4: Testing & Polish (1-2 days)
- [ ] Comprehensive testing
- [ ] Performance optimization
- [ ] Bug fixes and refinement

### Total Estimated Time: 6-10 days

---

*This migration guide provides a comprehensive path from legacy code to the modern shared components library.*

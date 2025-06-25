# Integration Guide - Shared Components Library

Step-by-step guide for integrating the shared components library into new projects.

## Table of Contents

- [Prerequisites](#prerequisites)
- [Build System Integration](#build-system-integration)
- [Basic Setup](#basic-setup)
- [Component Integration](#component-integration)
- [Advanced Configuration](#advanced-configuration)
- [Troubleshooting](#troubleshooting)

---

## Prerequisites

### Required Dependencies

1. **SDL3 Development Libraries**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install libsdl3-dev
   
   # Fedora/RedHat
   sudo dnf install SDL3-devel
   
   # macOS with Homebrew
   brew install sdl3
   
   # Windows - Download from https://github.com/libsdl-org/SDL/releases
   ```

2. **CMake 3.20 or later**
   ```bash
   cmake --version  # Check version
   ```

3. **C11 Compatible Compiler**
   - GCC 4.9+
   - Clang 3.3+ 
   - MSVC 2019+

### Directory Structure

```
your_project/
├── CMakeLists.txt
├── src/
│   └── main.c
├── shared/                    # Shared components library
│   ├── shared_components.h
│   ├── shared_components.c
│   ├── CMakeLists.txt
│   ├── text_renderer/
│   ├── ui_framework/
│   ├── palette_manager/
│   ├── sdl_framework/
│   └── utilities/
└── build/                     # Build directory
```

---

## Build System Integration

### CMake Configuration

#### Option 1: Subdirectory Integration (Recommended)

Add to your main `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.20)
project(YourProject)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find SDL3
find_package(SDL3 REQUIRED)

# Add shared components as subdirectory
add_subdirectory(shared)

# Create your executable
add_executable(your_app
    src/main.c
    # ... other source files
)

# Link libraries
target_link_libraries(your_app PRIVATE 
    shared_components 
    SDL3::SDL3
)

# Include directories
target_include_directories(your_app PRIVATE
    shared
    src
)
```

#### Option 2: External Library Integration

If shared components are in a separate directory:

```cmake
# Set path to shared components
set(SHARED_COMPONENTS_DIR "../shared-components-library")

# Add external library
add_subdirectory(${SHARED_COMPONENTS_DIR} shared_components_build)

# Link to your target
target_link_libraries(your_app PRIVATE shared_components)
target_include_directories(your_app PRIVATE ${SHARED_COMPONENTS_DIR})
```

#### Option 3: Find Package Integration

For system-wide installations:

```cmake
# Create FindSharedComponents.cmake module
find_package(SharedComponents REQUIRED)

target_link_libraries(your_app PRIVATE SharedComponents::shared_components)
```

### Build Commands

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the project
cmake --build .

# Install (optional)
cmake --install . --prefix /usr/local
```

---

## Basic Setup

### Minimal Integration Example

Create `src/main.c`:

```c
#include <shared_components.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    // Initialize shared components library
    if (!shared_components_init()) {
        fprintf(stderr, "Failed to initialize shared components\n");
        return 1;
    }
    
    printf("Shared Components Library v%s initialized\n", 
           shared_components_get_version());
    
    // Initialize SDL context
    SDLContext sdl_ctx;
    if (!sdl_init_context_simple(&sdl_ctx, "My Application", 800, 600)) {
        fprintf(stderr, "SDL initialization failed: %s\n", sdl_get_error());
        shared_components_cleanup();
        return 1;
    }
    
    // Initialize text renderer
    TextRenderer text_renderer;
    if (!text_renderer_init(&text_renderer, sdl_get_renderer(&sdl_ctx))) {
        fprintf(stderr, "Text renderer initialization failed\n");
        sdl_cleanup_context(&sdl_ctx);
        shared_components_cleanup();
        return 1;
    }
    
    // Main loop
    bool running = true;
    SDL_Event event;
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color black = {0, 0, 0, 255};
    
    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }
        
        // Render
        sdl_clear_screen(&sdl_ctx, black);
        text_render_string(&text_renderer, "Hello, Shared Components!", 
                          10, 10, white);
        sdl_present(&sdl_ctx);
    }
    
    // Cleanup
    text_renderer_cleanup(&text_renderer);
    sdl_cleanup_context(&sdl_ctx);
    shared_components_cleanup();
    
    return 0;
}
```

---

## Component Integration

### Text Rendering Integration

```c
#include <shared_components.h>

// Initialize text rendering
TextRenderer text_renderer;
text_renderer_init(&text_renderer, sdl_renderer);

// Set default color
SDL_Color default_color = {255, 255, 255, 255};
text_renderer_set_default_color(&text_renderer, default_color);

// Render text
text_render_string(&text_renderer, "Score: 1234", 10, 10, white);
text_render_string_default(&text_renderer, "Lives: 3", 10, 30);

// 7-segment display
SDL_Color red = {255, 0, 0, 255};
text_render_7segment_string(&text_renderer, "88:88", 100, 50, red, 2);

// Calculate text dimensions for centering
int text_width, text_height;
text_get_dimensions("GAME OVER", &text_width, &text_height);
int center_x = (screen_width - text_width) / 2;
text_render_string(&text_renderer, "GAME OVER", center_x, 200, red);
```

### UI Framework Integration

```c
#include <shared_components.h>

// Initialize button array
UIButtonArray buttons;
ui_button_array_init(&buttons, 10);

// Create buttons
UIButton start_button;
ui_button_init(&start_button, 100, 100, 120, 40, "Start Game");
ui_button_set_callback(&start_button, start_game_callback, game_state);

UIButton quit_button;
ui_button_init(&quit_button, 100, 150, 120, 40, "Quit");
ui_button_set_callback(&quit_button, quit_callback, NULL);

// Add to array
ui_button_array_add(&buttons, &start_button);
ui_button_array_add(&buttons, &quit_button);

// In event loop
if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    
    int clicked_button = ui_button_array_handle_input(&buttons, 
                                                     mouse_x, mouse_y, true);
    if (clicked_button >= 0) {
        printf("Button %d clicked\n", clicked_button);
    }
}

// Render buttons
ui_button_array_render(&buttons, sdl_renderer, &text_renderer);
```

### Palette Management Integration

```c
#include <shared_components.h>

// Initialize palette manager
PaletteManager palette;
palette_manager_init(&palette);

// Load palette from file
if (file_exists("game_palette.pal")) {
    if (!palette_manager_load(&palette, "game_palette.pal")) {
        printf("Failed to load palette, using defaults\n");
    }
}

// Use palette colors for rendering
SDL_Color bg_color = palette_get_sdl_color(&palette, 0);  // Background
SDL_Color fg_color = palette_get_sdl_color(&palette, 15); // Foreground

// Modify palette colors
RGBA new_color = palette_make_color(128, 64, 192, 255);
palette_set_color(&palette, 5, new_color);

// Save if modified
if (palette_manager_is_modified(&palette)) {
    palette_manager_save(&palette, NULL); // Save to current file
}
```

### Double-Click Detection Integration

```c
#include <shared_components.h>

// Initialize double-click detector
DoubleClickDetector double_click;
double_click_init(&double_click, DOUBLE_CLICK_THRESHOLD_NORMAL);

// In mouse event handling
if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
    int clicked_item = get_clicked_item(mouse_x, mouse_y);
    
    if (double_click_check(&double_click, clicked_item)) {
        printf("Double-click on item %d!\n", clicked_item);
        handle_double_click(clicked_item);
    } else {
        printf("Single click on item %d\n", clicked_item);
        handle_single_click(clicked_item);
    }
}
```

---

## Advanced Configuration

### Custom Initialization

```c
// Custom SDL context configuration
SDLContextConfig config = {
    .title = "My Game",
    .width = 1024,
    .height = 768,
    .resizable = true,
    .vsync = true,
    .fullscreen = false
};

SDLContext sdl_ctx;
sdl_init_context(&sdl_ctx, &config);

// Set logical presentation for consistent scaling
sdl_set_logical_presentation(&sdl_ctx, 800, 600);
```

### Error Handling Best Practices

```c
#include <shared_components.h>

bool initialize_application(AppState* app) {
    // Initialize shared components
    if (!shared_components_init()) {
        fprintf(stderr, "Shared components initialization failed\n");
        return false;
    }
    
    // Initialize SDL
    if (!sdl_init_context_simple(&app->sdl_ctx, "My App", 800, 600)) {
        fprintf(stderr, "SDL initialization failed: %s\n", sdl_get_error());
        goto cleanup_shared;
    }
    
    // Initialize text renderer
    if (!text_renderer_init(&app->text_renderer, 
                           sdl_get_renderer(&app->sdl_ctx))) {
        fprintf(stderr, "Text renderer initialization failed\n");
        goto cleanup_sdl;
    }
    
    // Initialize palette
    if (!palette_manager_init(&app->palette)) {
        fprintf(stderr, "Palette manager initialization failed\n");
        goto cleanup_text;
    }
    
    return true;
    
    // Cleanup chain for error handling
cleanup_text:
    text_renderer_cleanup(&app->text_renderer);
cleanup_sdl:
    sdl_cleanup_context(&app->sdl_ctx);
cleanup_shared:
    shared_components_cleanup();
    return false;
}

void cleanup_application(AppState* app) {
    palette_manager_save(&app->palette, NULL); // Save if needed
    text_renderer_cleanup(&app->text_renderer);
    sdl_cleanup_context(&app->sdl_ctx);
    shared_components_cleanup();
}
```

### Performance Optimization

```c
// Pre-calculate text dimensions for frequently used strings
typedef struct {
    const char* text;
    int width, height;
} TextMetrics;

TextMetrics ui_texts[] = {
    {"Score:", 0, 0},
    {"Lives:", 0, 0},
    {"Game Over", 0, 0}
};

void precalculate_text_metrics(void) {
    for (int i = 0; i < sizeof(ui_texts) / sizeof(ui_texts[0]); i++) {
        text_get_dimensions(ui_texts[i].text, 
                           &ui_texts[i].width, 
                           &ui_texts[i].height);
    }
}

// Batch button rendering
void render_ui_batch(UIButtonArray* buttons, SDL_Renderer* renderer, 
                     TextRenderer* text_renderer) {
    // Set render state once
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    
    // Render all buttons
    ui_button_array_render(buttons, renderer, text_renderer);
}
```

### Memory Management

```c
// Resource management structure
typedef struct {
    SDLContext* sdl_ctx;
    TextRenderer* text_renderer;
    PaletteManager* palette;
    UIButtonArray* buttons;
    DoubleClickDetector* double_click;
    bool initialized;
} AppResources;

bool init_app_resources(AppResources* res) {
    memset(res, 0, sizeof(AppResources));
    
    // Allocate structures
    res->sdl_ctx = malloc(sizeof(SDLContext));
    res->text_renderer = malloc(sizeof(TextRenderer));
    res->palette = malloc(sizeof(PaletteManager));
    res->buttons = malloc(sizeof(UIButtonArray));
    res->double_click = malloc(sizeof(DoubleClickDetector));
    
    if (!res->sdl_ctx || !res->text_renderer || !res->palette || 
        !res->buttons || !res->double_click) {
        cleanup_app_resources(res);
        return false;
    }
    
    // Initialize components
    if (!shared_components_init() ||
        !sdl_init_context_simple(res->sdl_ctx, "App", 800, 600) ||
        !text_renderer_init(res->text_renderer, 
                           sdl_get_renderer(res->sdl_ctx)) ||
        !palette_manager_init(res->palette) ||
        !ui_button_array_init(res->buttons, 10)) {
        cleanup_app_resources(res);
        return false;
    }
    
    double_click_init(res->double_click, DOUBLE_CLICK_THRESHOLD_NORMAL);
    res->initialized = true;
    return true;
}

void cleanup_app_resources(AppResources* res) {
    if (res->initialized) {
        if (res->text_renderer) text_renderer_cleanup(res->text_renderer);
        if (res->sdl_ctx) sdl_cleanup_context(res->sdl_ctx);
        if (res->buttons) ui_button_array_cleanup(res->buttons);
        shared_components_cleanup();
    }
    
    free(res->sdl_ctx);
    free(res->text_renderer);
    free(res->palette);
    free(res->buttons);
    free(res->double_click);
    
    memset(res, 0, sizeof(AppResources));
}
```

---

## Troubleshooting

### Common Issues and Solutions

#### 1. SDL3 Not Found
```
CMake Error: Could not find SDL3
```

**Solution**:
```cmake
# Add SDL3 path manually
set(SDL3_DIR "/path/to/SDL3/lib/cmake/SDL3")
find_package(SDL3 REQUIRED)

# Or use PkgConfig
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL3 REQUIRED sdl3)
```

#### 2. Shared Components Not Found
```
fatal error: shared_components.h: No such file or directory
```

**Solution**:
```cmake
# Ensure include directory is correct
target_include_directories(your_app PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/shared
)

# Check file structure
file(GLOB_RECURSE SHARED_FILES "${CMAKE_CURRENT_SOURCE_DIR}/shared/*")
message(STATUS "Shared files found: ${SHARED_FILES}")
```

#### 3. Linking Errors
```
undefined reference to `shared_components_init'
```

**Solution**:
```cmake
# Ensure library is linked
target_link_libraries(your_app PRIVATE shared_components)

# Check library was built
get_target_property(SHARED_LOCATION shared_components LOCATION)
message(STATUS "Shared library location: ${SHARED_LOCATION}")
```

#### 4. Runtime Initialization Failure
```c
// Add detailed error checking
if (!shared_components_init()) {
    fprintf(stderr, "Shared components init failed\n");
    exit(1);
}

if (!sdl_init_context_simple(&ctx, "App", 800, 600)) {
    fprintf(stderr, "SDL init failed: %s\n", SDL_GetError());
    shared_components_cleanup();
    exit(1);
}

// Verify initialization
if (!shared_components_is_initialized()) {
    fprintf(stderr, "Library not properly initialized\n");
    exit(1);
}
```

#### 5. Font Rendering Issues
```c
// Verify text renderer initialization
if (!text_renderer_is_ready(&text_renderer)) {
    fprintf(stderr, "Text renderer not ready\n");
    // Re-initialize
    text_renderer_cleanup(&text_renderer);
    text_renderer_init(&text_renderer, sdl_renderer);
}

// Check SDL renderer state
SDL_RendererInfo info;
SDL_GetRendererInfo(sdl_renderer, &info);
printf("Renderer: %s\n", info.name);
```

### Debug Configuration

```cmake
# Enable debug information
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -DDEBUG")

# Add sanitizers (GCC/Clang)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address -fsanitize=undefined")
```

### Validation Tools

```c
// Add validation calls in debug mode
#ifdef DEBUG
    // Validate shared components state
    assert(shared_components_is_initialized());
    
    // Validate SDL context
    assert(sdl_context_is_ready(&sdl_ctx));
    
    // Validate text renderer
    assert(text_renderer_is_ready(&text_renderer));
    
    // Validate font data
    font_validate_data();
#endif
```

---

## Complete Example Project

For a complete working example, see the `examples/` directory in the shared components repository. The example demonstrates:

- Full CMake integration
- All component usage
- Error handling
- Resource management
- Performance optimization techniques

```bash
# Clone and build example
git clone <repository-url>
cd shared-components-library/examples/complete-example
mkdir build && cd build
cmake ..
make
./complete_example
```

---

*This integration guide provides everything needed to successfully integrate the Shared Components Library into your project.*

# Shared Components Library - Examples

Practical code examples demonstrating how to use the shared components library.

## Examples Overview

| Example | Description | Complexity |
|---------|-------------|------------|
| [`basic_example.c`](#basic-example) | Minimal integration showing text rendering | Beginner |
| [`complete_example.c`](#complete-example) | Full-featured application using all components | Intermediate |
| [`text_demo.c`](#text-demo) | Text rendering and 7-segment display showcase | Beginner |
| [`ui_demo.c`](#ui-demo) | Interactive UI components demonstration | Intermediate |
| [`palette_editor.c`](#palette-editor) | Simple palette editor using shared components | Advanced |

## Running Examples

### Prerequisites

Ensure you have:
- SDL3 development libraries installed
- CMake 3.20 or later
- C11 compatible compiler
- Shared components library built

### Build Instructions

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ../shared/docs/examples

# Build all examples
cmake --build .

# Run specific example
./basic_example
./complete_example
./text_demo
./ui_demo
./palette_editor
```

---

## Basic Example

**File**: [`basic_example.c`](basic_example.c)

Minimal example showing library initialization and basic text rendering.

### Key Features Demonstrated
- Library initialization
- SDL context setup
- Text rendering
- Proper cleanup

### Code Highlights

```c
#include <shared_components.h>

int main() {
    // Initialize shared components
    if (!shared_components_init()) {
        return 1;
    }
    
    // Create SDL context
    SDLContext sdl_ctx;
    sdl_init_context_simple(&sdl_ctx, "Basic Example", 400, 300);
    
    // Initialize text renderer
    TextRenderer text_renderer;
    text_renderer_init(&text_renderer, sdl_get_renderer(&sdl_ctx));
    
    // Render some text
    SDL_Color white = {255, 255, 255, 255};
    text_render_string(&text_renderer, "Hello, World!", 10, 10, white);
    
    // Present and cleanup
    sdl_present(&sdl_ctx);
    SDL_Delay(2000);
    
    text_renderer_cleanup(&text_renderer);
    sdl_cleanup_context(&sdl_ctx);
    shared_components_cleanup();
    
    return 0;
}
```

---

## Complete Example

**File**: [`complete_example.c`](complete_example.c)

Comprehensive example using all shared components together.

### Key Features Demonstrated
- All component initialization
- Event handling
- Button interactions
- Palette management
- Double-click detection
- File operations

### Application Features
- Interactive buttons (Load, Save, Reset)
- Color palette display with click/double-click
- Status display with modification tracking
- Keyboard shortcuts
- Graceful error handling

---

## Text Demo

**File**: [`text_demo.c`](text_demo.c)

Showcase of text rendering capabilities including 5x7 font and 7-segment displays.

### Key Features Demonstrated
- Character set display (all 60 glyphs)
- 7-segment digit rendering
- Text alignment and positioning
- Color variations
- Performance measurement

### Visual Layout
```
┌─────────────────────────────────────┐
│ 5x7 Font Character Set Demo        │
│ 0123456789 ABCDEFGHIJKLMNOPQRSTUVWXYZ│
│ !@#$%^&*()[]{},.? ←↑→↓             │
│                                     │
│ 7-Segment Display Demo:             │
│ ██  ██  ██  ██  ██                  │
│   ██    ██    ██    ██              │
│ ██  ██  ██  ██  ██                  │
│                                     │
│ Performance: 1000 chars in 2.5ms   │
└─────────────────────────────────────┘
```

---

## UI Demo

**File**: [`ui_demo.c`](ui_demo.c)

Interactive demonstration of UI framework components.

### Key Features Demonstrated
- Button arrays with callbacks
- Hover and press states
- Custom button colors
- Mouse input handling
- UI layout patterns

### Interactive Elements
- **Color Buttons**: Change background color
- **Size Buttons**: Adjust UI element sizes
- **Action Buttons**: Demonstrate various callbacks
- **Toggle Buttons**: Show state management

---

## Palette Editor

**File**: [`palette_editor.c`](palette_editor.c)

Advanced example implementing a simple palette editor.

### Key Features Demonstrated
- Complete palette management workflow
- File load/save operations
- Color modification interface
- Undo/redo functionality (basic)
- Status tracking and user feedback

### User Interface
```
┌─────────────────────────────────────┐
│ Palette Editor - palette.pal [*]   │
├─────────────────────────────────────┤
│ [Load] [Save] [Reset] [Export]      │
├─────────────────────────────────────┤
│ Color Palette:                      │
│ ██ ██ ██ ██ ██ ██ ██ ██             │
│ ██ ██ ██ ██ ██ ██ ██ ██             │
├─────────────────────────────────────┤
│ Selected Color: #FF8040 (Index 5)  │
│ R: 255  G: 128  B: 64   A: 255     │
│ [+10] [+1] [-1] [-10] [Reset]      │
├─────────────────────────────────────┤
│ Status: Modified - 3 colors changed│
└─────────────────────────────────────┘
```

---

## Building Your Own Application

### Template Structure

Use this template as starting point for new applications:

```c
#include <shared_components.h>
#include <stdio.h>

typedef struct {
    SDLContext sdl_ctx;
    TextRenderer text_renderer;
    PaletteManager palette;
    UIButtonArray buttons;
    DoubleClickDetector double_click;
    
    // Application-specific state
    bool running;
    int current_mode;
} AppState;

bool init_application(AppState* app) {
    memset(app, 0, sizeof(AppState));
    
    if (!shared_components_init()) return false;
    if (!sdl_init_context_simple(&app->sdl_ctx, "My App", 800, 600)) goto error;
    if (!text_renderer_init(&app->text_renderer, sdl_get_renderer(&app->sdl_ctx))) goto error;
    if (!palette_manager_init(&app->palette)) goto error;
    if (!ui_button_array_init(&app->buttons, 10)) goto error;
    
    double_click_init(&app->double_click, DOUBLE_CLICK_THRESHOLD_NORMAL);
    app->running = true;
    
    return true;
    
error:
    cleanup_application(app);
    return false;
}

void cleanup_application(AppState* app) {
    ui_button_array_cleanup(&app->buttons);
    text_renderer_cleanup(&app->text_renderer);
    sdl_cleanup_context(&app->sdl_ctx);
    shared_components_cleanup();
}

void handle_events(AppState* app) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                app->running = false;
                break;
                
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                handle_mouse_input(app, &event);
                break;
                
            case SDL_EVENT_KEY_DOWN:
                handle_keyboard_input(app, &event);
                break;
        }
    }
}

void update_application(AppState* app) {
    // Update application logic
}

void render_application(AppState* app) {
    SDL_Color black = {0, 0, 0, 255};
    SDL_Color white = {255, 255, 255, 255};
    
    sdl_clear_screen(&app->sdl_ctx, black);
    
    // Render UI
    ui_button_array_render(&app->buttons, sdl_get_renderer(&app->sdl_ctx), 
                          &app->text_renderer);
    
    // Render text
    text_render_string(&app->text_renderer, "My Application", 10, 10, white);
    
    sdl_present(&app->sdl_ctx);
}

int main() {
    AppState app;
    
    if (!init_application(&app)) {
        fprintf(stderr, "Application initialization failed\n");
        return 1;
    }
    
    while (app.running) {
        handle_events(&app);
        update_application(&app);
        render_application(&app);
        
        SDL_Delay(16); // ~60 FPS
    }
    
    cleanup_application(&app);
    return 0;
}
```

### Best Practices

1. **Error Handling**: Always check return values and handle failures gracefully
2. **Resource Management**: Use RAII-style initialization/cleanup patterns
3. **Component Lifecycle**: Initialize components in dependency order
4. **Performance**: Cache frequently used values and minimize API calls
5. **Memory Safety**: Validate pointers and array bounds
6. **User Experience**: Provide visual feedback for all interactions

### Common Patterns

#### Button Callback Pattern
```c
void button_callback(void* userdata) {
    AppState* app = (AppState*)userdata;
    // Handle button click
}

UIButton btn;
ui_button_init(&btn, x, y, w, h, "Click Me");
ui_button_set_callback(&btn, button_callback, app);
```

#### Color Selection Pattern
```c
void handle_color_click(AppState* app, int color_index) {
    if (double_click_check(&app->double_click, color_index)) {
        // Double-click: Edit color
        edit_color_dialog(app, color_index);
    } else {
        // Single click: Select color
        app->selected_color = color_index;
    }
}
```

#### File Operation Pattern
```c
bool save_application_data(AppState* app, const char* filename) {
    // Save palette
    if (!palette_manager_save(&app->palette, filename)) {
        show_error_message("Failed to save palette");
        return false;
    }
    
    // Save additional application data
    // ... custom save logic
    
    return true;
}
```

---

*These examples provide practical starting points for using the shared components library in your own applications.*

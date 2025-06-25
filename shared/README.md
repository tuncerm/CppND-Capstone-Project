# Shared Components Library

A reusable C library extracted from palette-maker and tile-maker applications, providing unified text rendering, UI framework, palette management, SDL3 integration, and common utilities.

## 🚀 Features

- **5x7 Bitmap Font System**: Complete text rendering with 60 character glyphs
- **7-Segment Display Support**: Digital display-style text rendering with scaling
- **UI Framework**: Buttons, primitives, and mouse interaction handling
- **Palette Management**: 16-color RGBA palette system with file I/O
- **SDL3 Integration**: Streamlined context management and utilities
- **Common Utilities**: Double-click detection, file operations, and more

## 📋 Quick Start

### Prerequisites

- SDL3 development libraries
- CMake 3.20 or later
- C11 compatible compiler

### Basic Integration

```c
#include <shared_components.h>

int main() {
    // Initialize the library
    if (!shared_components_init()) {
        fprintf(stderr, "Failed to initialize shared components\n");
        return 1;
    }
    
    // Create SDL context
    SDLContext ctx;
    if (!sdl_init_context_simple(&ctx, "My App", 800, 600)) {
        fprintf(stderr, "Failed to initialize SDL\n");
        return 1;
    }
    
    // Initialize text renderer
    TextRenderer text_renderer;
    text_renderer_init(&text_renderer, sdl_get_renderer(&ctx));
    
    // Render some text
    SDL_Color white = {255, 255, 255, 255};
    text_render_string(&text_renderer, "Hello, World!", 10, 10, white);
    
    // Present and cleanup
    sdl_present(&ctx);
    
    text_renderer_cleanup(&text_renderer);
    sdl_cleanup_context(&ctx);
    shared_components_cleanup();
    
    return 0;
}
```

### CMake Integration

```cmake
# Find and link the shared components library
target_link_libraries(your_target PRIVATE shared_components SDL3::SDL3)
target_include_directories(your_target PRIVATE path/to/shared)
```

## 📚 Documentation

| Document | Description |
|----------|-------------|
| [API Reference](docs/API.md) | Complete API documentation for all components |
| [Integration Guide](docs/INTEGRATION.md) | Step-by-step integration instructions |
| [Migration Guide](docs/MIGRATION.md) | Migrating from palette-maker/tile-maker code |
| [Architecture](docs/ARCHITECTURE.md) | Design decisions and system architecture |
| [Examples](docs/examples/) | Code examples and usage patterns |

## 🏗️ Library Components

### Text Renderer
- **Files**: [`text_renderer.h`](text_renderer/text_renderer.h), [`font_data.h`](text_renderer/font_data.h)
- **Features**: 5x7 bitmap font, 7-segment displays, character mapping
- **Usage**: Text rendering for UI labels, debug output, digital displays

### UI Framework  
- **Files**: [`ui_button.h`](ui_framework/ui_button.h), [`ui_primitives.h`](ui_framework/ui_primitives.h)
- **Features**: Interactive buttons, rectangle rendering, coordinate utilities
- **Usage**: Building interactive user interfaces

### Palette Manager
- **Files**: [`palette_manager.h`](palette_manager/palette_manager.h)
- **Features**: 16-color RGBA palettes, file I/O, modification tracking
- **Usage**: Color management for graphics applications

### SDL Framework
- **Files**: [`sdl_context.h`](sdl_framework/sdl_context.h)
- **Features**: SDL3 initialization, window management, rendering utilities
- **Usage**: Simplified SDL3 setup and management

### Utilities
- **Files**: [`double_click.h`](utilities/double_click.h), [`file_utils.h`](utilities/file_utils.h)
- **Features**: Double-click detection, file operations, path manipulation
- **Usage**: Common functionality needed across applications

## 🔧 Build Information

**Version**: 1.0.0  
**C Standard**: C11  
**Dependencies**: SDL3  
**Build System**: CMake  

## 📝 Version History

### v1.0.0 (Current)
- Initial release with components extracted from palette-maker and tile-maker
- Complete 5x7 bitmap font system with 60 character glyphs
- 7-segment display rendering support
- Unified button and UI primitive system
- Comprehensive palette management with file I/O
- SDL3 context management framework
- Double-click detection and file utilities

## 🤝 Contributing

This library was extracted from existing applications to promote code reuse. When adding features:

1. Maintain API compatibility
2. Follow existing naming conventions
3. Update documentation
4. Test with both palette-maker and tile-maker
5. Ensure C11 compatibility

## 📄 License

Same license as parent project - see project root for details.

## 🔗 Related Projects

- **palette-maker**: 16-color palette editor using this library
- **tile-maker**: Tile/sprite editor using this library

---

*Generated from shared component extraction - ROO#SUB_PARENT-DOCUMENT_S003_20250625230022_G9E4F5*

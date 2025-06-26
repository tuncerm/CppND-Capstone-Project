# Shared Components Testing Framework

Comprehensive testing suite for the shared_components library using Google Test framework.

## 🎯 Overview

This testing framework provides:
- **Unit Tests**: Individual component testing with 90%+ coverage
- **Integration Tests**: Cross-component interaction validation
- **Performance Tests**: Benchmarking critical operations
- **Error Handling Tests**: Robustness validation
- **Platform Compatibility**: Windows, Linux, macOS support

## 📁 Test Structure

```
tests/
├── CMakeLists.txt              # Test build configuration
├── test_main.cpp               # Test entry point with SDL setup
├── utils/                      # Test utilities and helpers
│   ├── test_helpers.h          # Common test utilities
│   └── test_helpers.cpp        # Helper implementations
├── unit/                       # Unit tests for individual components
│   ├── test_shared_components.cpp    # Core library tests
│   ├── test_text_renderer.cpp       # Text rendering tests
│   ├── test_ui_button.cpp           # UI framework tests
│   ├── test_palette_manager.cpp     # Palette management tests
│   ├── test_sdl_context.cpp         # SDL integration tests
│   ├── test_file_utils.cpp          # File utilities tests
│   └── test_double_click.cpp        # Input handling tests
└── integration/                # Integration tests
    ├── test_text_ui_integration.cpp        # Text + UI integration
    ├── test_palette_rendering_integration.cpp  # Palette + rendering
    └── test_sdl_components_integration.cpp     # Full system integration
```

## 🚀 Quick Start

### Prerequisites

- CMake 3.20 or later
- C++17 compatible compiler
- SDL3 development libraries
- Google Test (automatically downloaded if not found)

### Building and Running Tests

```bash
# Configure with tests enabled
mkdir build && cd build
cmake .. -DBUILD_TESTING=ON

# Build tests
cmake --build . --target shared_components_tests

# Run all tests
ctest --verbose

# Or run test executable directly
./bin/shared_components_tests
```

### Windows (Visual Studio)

```cmd
mkdir build && cd build
cmake .. -DBUILD_TESTING=ON -G "Visual Studio 17 2022"
cmake --build . --config Debug --target shared_components_tests
ctest -C Debug --verbose
```

## 📊 Test Categories

### Unit Tests

Each component has comprehensive unit tests covering:

- **Initialization/Cleanup**: Proper setup and teardown
- **Core Functionality**: All public API functions
- **Error Handling**: Invalid inputs and edge cases
- **State Management**: Internal state consistency
- **Performance**: Critical path benchmarks

### Integration Tests

Cross-component tests ensuring:

- **Data Flow**: Information passing between components
- **Resource Sharing**: SDL context sharing
- **Color Consistency**: Palette integration with rendering
- **Input Processing**: Event handling across UI components
- **File I/O**: Persistence and loading workflows

### Performance Tests

Benchmarking tests for:

- Rendering operations (text, UI, graphics)
- Palette access and modification
- File I/O operations
- Memory allocation patterns
- Input processing latency

## 🛠️ Test Configuration

### CMake Options

```cmake
# Enable/disable tests (default: ON when building main project)
-DBUILD_TESTING=ON/OFF

# Enable/disable specific test categories
-DSHARED_COMPONENTS_BUILD_TESTS=ON/OFF

# Google Test configuration
-DGTEST_FORCE_SHARED_CRT=ON  # Windows MSVC compatibility
```

### Test Filtering

Run specific test suites:

```bash
# Run only unit tests
./shared_components_tests --gtest_filter="*UnitTest*"

# Run specific component tests
./shared_components_tests --gtest_filter="*TextRenderer*"

# Run integration tests only
./shared_components_tests --gtest_filter="*Integration*"

# Exclude performance tests
./shared_components_tests --gtest_filter="-*Performance*"
```

## 📈 Coverage Analysis

### GCC/Clang Coverage

```bash
# Build with coverage
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
make coverage

# View results
open coverage/index.html
```

### Coverage Reports

The testing framework generates coverage reports for:

- **Line Coverage**: Percentage of code lines executed
- **Function Coverage**: Percentage of functions called
- **Branch Coverage**: Percentage of conditional branches taken
- **Component Coverage**: Per-component breakdown

**Target Coverage**: 90%+ for all components

## 🔧 Test Utilities

### Test Fixtures

- **`SDLTestFixture`**: Provides SDL context for rendering tests
- **`TextRendererTestFixture`**: Pre-configured text renderer
- **`PaletteManagerTestFixture`**: Initialized palette manager

### Helper Functions

- **`CreateTestButton()`**: Standard button configuration
- **`CreateTestPalette()`**: Known color palette for testing
- **`CreateTempTestFile()`**: Temporary file management
- **`VerifySDLContext()`**: Context validation

### Test Macros

- **`EXPECT_RGBA_EQ()`**: Color component comparison
- **`EXPECT_SDL_COLOR_EQ()`**: SDL color comparison
- **`ASSERT_SDL_SUCCESS()`**: SDL operation validation

## 🐛 Debugging Tests

### Visual Debugging

Tests create hidden SDL windows by default. For visual debugging:

```cpp
// In test setup, show windows for visual inspection
SDL_ShowWindow(sdl_get_window(&sdl_context));
```

### Memory Debugging

#### Valgrind (Linux)

```bash
make valgrind  # Runs tests under Valgrind
```

#### AddressSanitizer

```bash
cmake .. -DCMAKE_CXX_FLAGS="-fsanitize=address -g"
```

### Test Output

```bash
# Verbose output
./shared_components_tests --gtest_print_time=1

# XML output for CI
./shared_components_tests --gtest_output=xml:test_results.xml
```

## 🔄 Continuous Integration

### GitHub Actions Example

```yaml
- name: Build and Test
  run: |
    mkdir build && cd build
    cmake .. -DBUILD_TESTING=ON
    cmake --build .
    ctest --output-on-failure
```

### Test Status

Tests automatically run on:
- ✅ Push to main branch
- ✅ Pull requests
- ✅ Release builds
- ✅ Nightly builds

## 📋 Test Checklist

### Adding New Tests

When adding new functionality:

1. **Unit Tests**: Test the component in isolation
2. **Integration Tests**: Test with related components
3. **Error Tests**: Test failure conditions
4. **Performance Tests**: Benchmark if performance-critical
5. **Documentation**: Update test documentation

### Test Quality Guidelines

- **Descriptive Names**: Clear test and assertion descriptions
- **Independent Tests**: No dependencies between test cases
- **Deterministic**: Same results every run
- **Fast Execution**: Unit tests should complete quickly
- **Good Coverage**: Aim for 90%+ line coverage

## 🚨 Known Issues

### Platform-Specific

- **Windows**: SDL3 DLL path issues in debug builds
- **macOS**: Framework linking with CMake < 3.21
- **Linux**: X11 display requirement for SDL tests

### Workarounds

- Use `SDL_VIDEODRIVER=dummy` for headless testing
- Set appropriate library paths for SDL3
- Configure Xvfb for headless Linux CI

## 📞 Support

For testing issues:

1. Check test output for specific failure details
2. Verify SDL3 installation and library paths
3. Ensure Google Test compatibility with your compiler
4. Review platform-specific build requirements

## 🔄 Version History

- **v1.0.0**: Initial testing framework
  - Complete unit test coverage
  - Integration tests for all components
  - Performance benchmarking
  - Cross-platform compatibility
  - Automated CI/CD integration

---

*Testing Framework for Shared Components Library v1.0.0*

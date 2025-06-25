/**
 * Text Demo - Shared Components Library
 *
 * Comprehensive demonstration of text rendering capabilities:
 * - 5x7 bitmap font character set
 * - 7-segment display rendering
 * - Text alignment and positioning
 * - Performance measurement
 */

#include <shared_components.h>
#include <stdio.h>
#include <string.h>

// Demo sections
typedef enum {
    DEMO_CHARACTER_SET,
    DEMO_7_SEGMENT,
    DEMO_ALIGNMENT,
    DEMO_PERFORMANCE,
    DEMO_COUNT
} DemoSection;

typedef struct {
    SDLContext sdl_ctx;
    TextRenderer text_renderer;
    DemoSection current_section;
    Uint64 last_switch_time;
    float performance_time;
    bool running;
} TextDemo;

bool init_demo(TextDemo* demo) {
    memset(demo, 0, sizeof(TextDemo));

    if (!shared_components_init())
        return false;
    if (!sdl_init_context_simple(&demo->sdl_ctx, "Text Rendering Demo", 800, 600))
        return false;
    if (!text_renderer_init(&demo->text_renderer, sdl_get_renderer(&demo->sdl_ctx)))
        return false;

    demo->current_section = DEMO_CHARACTER_SET;
    demo->last_switch_time = SDL_GetTicks();
    demo->running = true;

    return true;
}

void cleanup_demo(TextDemo* demo) {
    text_renderer_cleanup(&demo->text_renderer);
    sdl_cleanup_context(&demo->sdl_ctx);
    shared_components_cleanup();
}

void render_character_set_demo(TextDemo* demo) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color yellow = {255, 255, 0, 255};
    SDL_Color cyan = {0, 255, 255, 255};

    // Title
    text_render_string(&demo->text_renderer, "5x7 Font Character Set Demo", 10, 10, yellow);

    // Digits
    text_render_string(&demo->text_renderer, "Digits:", 10, 40, cyan);
    text_render_string(&demo->text_renderer, "0123456789", 10, 60, white);

    // Uppercase letters
    text_render_string(&demo->text_renderer, "Letters:", 10, 90, cyan);
    text_render_string(&demo->text_renderer, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 10, 110, white);

    // Punctuation and special characters
    text_render_string(&demo->text_renderer, "Punctuation:", 10, 140, cyan);
    text_render_string(&demo->text_renderer, "!@#$%^&*()[]{},.?:;", 10, 160, white);

    // Arrows and special symbols
    text_render_string(&demo->text_renderer, "Special:", 10, 190, cyan);
    text_render_string(&demo->text_renderer, "Arrow Keys: <>^v", 10, 210, white);

    // Character dimensions info
    int char_width = FONT_WIDTH;
    int char_height = FONT_HEIGHT;
    int char_spacing = CHAR_SPACING;

    char info[128];
    snprintf(info, sizeof(info), "Char Size: %dx%d, Spacing: %d", char_width, char_height,
             char_spacing);
    text_render_string(&demo->text_renderer, info, 10, 250, cyan);

    // Total glyphs
    snprintf(info, sizeof(info), "Total Glyphs: %d", GLYPH_COUNT);
    text_render_string(&demo->text_renderer, info, 10, 270, cyan);

    // Instructions
    text_render_string(&demo->text_renderer, "Press SPACE to switch demos", 10, 550, yellow);
}

void render_7_segment_demo(TextDemo* demo) {
    SDL_Color red = {255, 0, 0, 255};
    SDL_Color green = {0, 255, 0, 255};
    SDL_Color blue = {0, 0, 255, 255};
    SDL_Color yellow = {255, 255, 0, 255};
    SDL_Color white = {255, 255, 255, 255};

    // Title
    text_render_string(&demo->text_renderer, "7-Segment Display Demo", 10, 10, yellow);

    // Clock display
    text_render_string(&demo->text_renderer, "Digital Clock:", 10, 50, white);

    // Get current time for demo
    Uint64 current_time = SDL_GetTicks();
    int seconds = (current_time / 1000) % 60;
    int minutes = (current_time / 60000) % 60;
    int hours = (current_time / 3600000) % 24;

    char time_str[16];
    snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", hours, minutes, seconds);
    text_render_7segment_string(&demo->text_renderer, time_str, 50, 80, red, 2);

    // Large single digits
    text_render_string(&demo->text_renderer, "Large Digits:", 10, 180, white);
    for (int i = 0; i < 10; i++) {
        char digit = '0' + i;
        int x = 50 + (i % 5) * 60;
        int y = 210 + (i / 5) * 80;

        SDL_Color color = (i % 3 == 0) ? red : (i % 3 == 1) ? green : blue;
        text_render_7segment_digit(&demo->text_renderer, digit, x, y, color, 1);
    }

    // Scale demonstration
    text_render_string(&demo->text_renderer, "Different Scales:", 10, 380, white);
    text_render_7segment_string(&demo->text_renderer, "888", 50, 410, green, 1);
    text_render_7segment_string(&demo->text_renderer, "888", 150, 410, green, 2);
    text_render_7segment_string(&demo->text_renderer, "888", 300, 410, green, 3);

    // Dimensions info
    int width, height;
    text_get_7segment_dimensions("888", 2, &width, &height);
    char info[64];
    snprintf(info, sizeof(info), "Scale 2 size: %dx%d pixels", width, height);
    text_render_string(&demo->text_renderer, info, 10, 500, white);

    text_render_string(&demo->text_renderer, "Press SPACE to switch demos", 10, 550, yellow);
}

void render_alignment_demo(TextDemo* demo) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color yellow = {255, 255, 0, 255};
    SDL_Color cyan = {0, 255, 255, 255};
    SDL_Color red = {255, 0, 0, 255};

    // Title
    text_render_string(&demo->text_renderer, "Text Alignment Demo", 10, 10, yellow);

    // Left alignment
    text_render_string(&demo->text_renderer, "Left Aligned:", 10, 50, cyan);
    text_render_string(&demo->text_renderer, "This is left aligned text", 10, 70, white);
    text_render_string(&demo->text_renderer, "Short text", 10, 90, white);
    text_render_string(&demo->text_renderer, "A very long line of text here", 10, 110, white);

    // Center alignment (manual calculation)
    text_render_string(&demo->text_renderer, "Center Aligned:", 10, 150, cyan);

    const char* center_texts[] = {"Centered text", "Short", "This is a longer centered line"};

    int screen_width = 800;
    for (int i = 0; i < 3; i++) {
        int text_width, text_height;
        text_get_dimensions(center_texts[i], &text_width, &text_height);
        int center_x = (screen_width - text_width) / 2;
        int y = 170 + i * 20;
        text_render_string(&demo->text_renderer, center_texts[i], center_x, y, white);
    }

    // Right alignment (manual calculation)
    text_render_string(&demo->text_renderer, "Right Aligned:", 10, 250, cyan);

    const char* right_texts[] = {"Right aligned text", "Short",
                                 "This is a longer right aligned line"};

    for (int i = 0; i < 3; i++) {
        int text_width, text_height;
        text_get_dimensions(right_texts[i], &text_width, &text_height);
        int right_x = screen_width - text_width - 10;
        int y = 270 + i * 20;
        text_render_string(&demo->text_renderer, right_texts[i], right_x, y, white);
    }

    // Grid alignment
    text_render_string(&demo->text_renderer, "Grid Layout:", 10, 350, cyan);

    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 4; col++) {
            char cell_text[16];
            snprintf(cell_text, sizeof(cell_text), "R%dC%d", row + 1, col + 1);

            int x = 50 + col * 80;
            int y = 370 + row * 25;
            text_render_string(&demo->text_renderer, cell_text, x, y, white);
        }
    }

    // Baseline alignment info
    text_render_string(&demo->text_renderer, "Font Height: 7 pixels", 10, 470, red);
    text_render_string(&demo->text_renderer, "Character Width: 5 pixels", 10, 490, red);
    text_render_string(&demo->text_renderer, "Character Spacing: 6 pixels", 10, 510, red);

    text_render_string(&demo->text_renderer, "Press SPACE to switch demos", 10, 550, yellow);
}

void render_performance_demo(TextDemo* demo) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color yellow = {255, 255, 0, 255};
    SDL_Color green = {0, 255, 0, 255};
    SDL_Color red = {255, 0, 0, 255};

    // Title
    text_render_string(&demo->text_renderer, "Performance Demo", 10, 10, yellow);

    // Measure text rendering performance
    const int test_iterations = 1000;
    const char* test_text = "Performance Test String";

    Uint64 start_time = SDL_GetPerformanceCounter();

    for (int i = 0; i < test_iterations; i++) {
        // Render text off-screen for performance measurement
        text_render_string(&demo->text_renderer, test_text, -100, -100, white);
    }

    Uint64 end_time = SDL_GetPerformanceCounter();
    double frequency = (double)SDL_GetPerformanceFrequency();
    demo->performance_time = (float)((end_time - start_time) / frequency * 1000.0);

    // Display performance results
    char perf_info[128];
    snprintf(perf_info, sizeof(perf_info), "Rendered %d strings in %.2f ms", test_iterations,
             demo->performance_time);
    text_render_string(&demo->text_renderer, perf_info, 10, 50, green);

    float strings_per_ms = test_iterations / demo->performance_time;
    snprintf(perf_info, sizeof(perf_info), "Performance: %.1f strings/ms", strings_per_ms);
    text_render_string(&demo->text_renderer, perf_info, 10, 70, green);

    // Performance rating
    const char* rating;
    SDL_Color rating_color;
    if (strings_per_ms > 1000) {
        rating = "Excellent";
        rating_color = green;
    } else if (strings_per_ms > 500) {
        rating = "Good";
        rating_color = yellow;
    } else {
        rating = "Needs Optimization";
        rating_color = red;
    }

    snprintf(perf_info, sizeof(perf_info), "Rating: %s", rating);
    text_render_string(&demo->text_renderer, perf_info, 10, 90, rating_color);

    // Stress test display
    text_render_string(&demo->text_renderer, "Live Stress Test:", 10, 130, white);

    // Render many strings for visual stress test
    for (int i = 0; i < 20; i++) {
        char stress_text[32];
        snprintf(stress_text, sizeof(stress_text), "Line %02d: Stress test text", i + 1);

        SDL_Color color = {(Uint8)(128 + i * 6), (Uint8)(255 - i * 8), (Uint8)(100 + i * 4), 255};

        text_render_string(&demo->text_renderer, stress_text, 10, 150 + i * 15, color);
    }

    text_render_string(&demo->text_renderer, "Press SPACE to switch demos", 10, 550, yellow);
}

void handle_events(TextDemo* demo) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                demo->running = false;
                break;

            case SDL_EVENT_KEY_DOWN:
                if (event.key.keysym.sym == SDLK_SPACE) {
                    demo->current_section = (demo->current_section + 1) % DEMO_COUNT;
                    demo->last_switch_time = SDL_GetTicks();
                } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                    demo->running = false;
                }
                break;
        }
    }
}

void update_demo(TextDemo* demo) {
    // Auto-switch sections every 5 seconds
    Uint64 current_time = SDL_GetTicks();
    if (current_time - demo->last_switch_time > 5000) {
        demo->current_section = (demo->current_section + 1) % DEMO_COUNT;
        demo->last_switch_time = current_time;
    }
}

void render_demo(TextDemo* demo) {
    SDL_Color black = {0, 0, 0, 255};
    sdl_clear_screen(&demo->sdl_ctx, black);

    switch (demo->current_section) {
        case DEMO_CHARACTER_SET:
            render_character_set_demo(demo);
            break;
        case DEMO_7_SEGMENT:
            render_7_segment_demo(demo);
            break;
        case DEMO_ALIGNMENT:
            render_alignment_demo(demo);
            break;
        case DEMO_PERFORMANCE:
            render_performance_demo(demo);
            break;
        default:
            break;
    }

    sdl_present(&demo->sdl_ctx);
}

int main(int argc, char* argv[]) {
    printf("Text Rendering Demo - Shared Components Library\n");
    printf("Controls:\n");
    printf("  SPACE - Switch demo sections\n");
    printf("  ESC   - Exit\n");
    printf("  Auto-switch every 5 seconds\n\n");

    TextDemo demo;

    if (!init_demo(&demo)) {
        fprintf(stderr, "Failed to initialize demo\n");
        return 1;
    }

    while (demo.running) {
        handle_events(&demo);
        update_demo(&demo);
        render_demo(&demo);

        SDL_Delay(16);  // ~60 FPS
    }

    cleanup_demo(&demo);
    printf("Demo completed successfully!\n");

    return 0;
}

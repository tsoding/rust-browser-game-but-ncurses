#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#include "game.h"

#define DISPLAY_WIDTH 800
#define DISPLAY_HEIGHT 600
#define CONSOLE_COLS 100
#define CONSOLE_ROWS 40
#define CHAR_WIDTH_IN_PIXELS (DISPLAY_WIDTH / CONSOLE_COLS)
#define CHAR_HEIGHT_IN_PIXELS (DISPLAY_HEIGHT / CONSOLE_ROWS)

static_assert(DISPLAY_WIDTH % CONSOLE_COLS == 0,
              "Display width should divisible by console columns");
static_assert(DISPLAY_HEIGHT % CONSOLE_ROWS == 0,
              "Display height should divisible by console rows");

#define ABGR_ALPHA(pixel) ((pixel >> (3 * 8)) & 0xFF)
#define ABGR_BLUE(pixel) ((pixel >> (2 * 8)) & 0xFF)
#define ABGR_GREEN(pixel) ((pixel >> (1 * 8)) & 0xFF)
#define ABGR_RED(pixel) ((pixel >> (0 * 8)) & 0xFF)

uint8_t abgr_bright(uint32_t pixel)
{
    const uint8_t r = ABGR_RED(pixel);
    const uint8_t g = ABGR_GREEN(pixel);
    const uint8_t b = ABGR_BLUE(pixel);

    uint8_t result = r;
    if (g > result) result = g;
    if (b > result) result = b;

    return result;
}

const char bright_chars[] = {' ', '.', 'a', 'A', '#'};
#define BRIGHT_CHARS_COUNT (sizeof(bright_chars) / sizeof(bright_chars[0]))
static_assert(UINT8_MAX % BRIGHT_CHARS_COUNT == 0,
              "We want the brightness of the pixels to be "
              "divisible by the amount of brightness chars");

void pixels_to_chars(char *chars, uint32_t *pixels)
{
    for (int row = 0; row < CONSOLE_ROWS; ++row) {
        for (int col = 0; col < CONSOLE_COLS; ++col) {
            uint32_t bright_sum = 0;
            for (int y = 0; y < CHAR_HEIGHT_IN_PIXELS; ++y) {
                for (int x = 0; x < CHAR_WIDTH_IN_PIXELS; ++x) {
                    const size_t pixel_x = col * CHAR_WIDTH_IN_PIXELS;
                    const size_t pixel_y = row * CHAR_HEIGHT_IN_PIXELS;
                    const size_t index = pixel_y * DISPLAY_WIDTH + pixel_x;
                    bright_sum += abgr_bright(pixels[index]);
                }
            }
            bright_sum /= CHAR_WIDTH_IN_PIXELS * CHAR_HEIGHT_IN_PIXELS;
            assert(bright_sum <= UINT8_MAX);
            const uint8_t t = 255 / BRIGHT_CHARS_COUNT;
            const uint8_t bright_index = bright_sum / t;
            assert(bright_index < BRIGHT_CHARS_COUNT);
            chars[row * CONSOLE_COLS + col] = bright_chars[bright_index];
        }
    }
}

void print_chars(char *chars)
{
    for (size_t row = 0; row < CONSOLE_ROWS; ++row) {
        for (size_t col = 0; col < CONSOLE_COLS; ++col) {
            fputc(chars[row * CONSOLE_COLS + col], stdout);
        }
        fputc('\n', stdout);
    }
}

int main()
{
    static char chars[CONSOLE_ROWS * CONSOLE_COLS] = {0};

    init();

    const size_t display_width = get_display_width();
    const size_t display_height = get_display_height();
    assert(display_width == DISPLAY_WIDTH);
    assert(display_height == DISPLAY_HEIGHT);

    uint32_t *pixels = get_display();
    next_frame(0.0f);
    printf("Display %zux%zu at %p\n", display_width, display_height, pixels);

    pixels_to_chars(chars, pixels);
    print_chars(chars);

    while (1) {
        next_frame(1.0f / 60.0f);
        pixels_to_chars(chars, pixels);
        print_chars(chars);
    }

    return 0;
}

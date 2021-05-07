#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <curses.h>

#include <unistd.h>
#include <locale.h>

#include "game.h"

#define DISPLAY_WIDTH 800
#define DISPLAY_HEIGHT 600
#define CONSOLE_COLS 100
#define CONSOLE_ROWS 50
static_assert(DISPLAY_WIDTH % CONSOLE_COLS == 0, "");
static_assert(DISPLAY_HEIGHT % CONSOLE_ROWS == 0, "");

#define CHAR_WIDTH_IN_PIXELS (DISPLAY_WIDTH / CONSOLE_COLS)
#define CHAR_HEIGHT_IN_PIXELS (DISPLAY_HEIGHT / CONSOLE_ROWS)
#define BRAILLE_DOTS_COLS 2
#define BRAILLE_DOTS_ROWS 4
#define BRAILLE_DOTS_COUNT (BRAILLE_DOTS_ROWS * BRAILLE_DOTS_COLS)
static_assert(CHAR_WIDTH_IN_PIXELS % BRAILLE_DOTS_COLS == 0, "");
static_assert(CHAR_HEIGHT_IN_PIXELS % BRAILLE_DOTS_ROWS == 0, "");

#define BRAILLE_DOT_WIDTH_IN_PIXELS (CHAR_WIDTH_IN_PIXELS / BRAILLE_DOTS_COLS)
#define BRAILLE_DOT_HEIGHT_IN_PIXELS (CHAR_HEIGHT_IN_PIXELS / BRAILLE_DOTS_ROWS)

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

typedef struct {
    char bytes[3];
} Braille;

#define BRAILLES_COUNT 256
const Braille brailles[BRAILLES_COUNT] = {
    {.bytes = "⠀"},
    {.bytes = "⠁"},
    {.bytes = "⠂"},
    {.bytes = "⠃"},
    {.bytes = "⠄"},
    {.bytes = "⠅"},
    {.bytes = "⠆"},
    {.bytes = "⠇"},
    {.bytes = "⠈"},
    {.bytes = "⠉"},
    {.bytes = "⠊"},
    {.bytes = "⠋"},
    {.bytes = "⠌"},
    {.bytes = "⠍"},
    {.bytes = "⠎"},
    {.bytes = "⠏"},
    {.bytes = "⠐"},
    {.bytes = "⠑"},
    {.bytes = "⠒"},
    {.bytes = "⠓"},
    {.bytes = "⠔"},
    {.bytes = "⠕"},
    {.bytes = "⠖"},
    {.bytes = "⠗"},
    {.bytes = "⠘"},
    {.bytes = "⠙"},
    {.bytes = "⠚"},
    {.bytes = "⠛"},
    {.bytes = "⠜"},
    {.bytes = "⠝"},
    {.bytes = "⠞"},
    {.bytes = "⠟"},
    {.bytes = "⠠"},
    {.bytes = "⠡"},
    {.bytes = "⠢"},
    {.bytes = "⠣"},
    {.bytes = "⠤"},
    {.bytes = "⠥"},
    {.bytes = "⠦"},
    {.bytes = "⠧"},
    {.bytes = "⠨"},
    {.bytes = "⠩"},
    {.bytes = "⠪"},
    {.bytes = "⠫"},
    {.bytes = "⠬"},
    {.bytes = "⠭"},
    {.bytes = "⠮"},
    {.bytes = "⠯"},
    {.bytes = "⠰"},
    {.bytes = "⠱"},
    {.bytes = "⠲"},
    {.bytes = "⠳"},
    {.bytes = "⠴"},
    {.bytes = "⠵"},
    {.bytes = "⠶"},
    {.bytes = "⠷"},
    {.bytes = "⠸"},
    {.bytes = "⠹"},
    {.bytes = "⠺"},
    {.bytes = "⠻"},
    {.bytes = "⠼"},
    {.bytes = "⠽"},
    {.bytes = "⠾"},
    {.bytes = "⠿"},
    {.bytes = "⡀"},
    {.bytes = "⡁"},
    {.bytes = "⡂"},
    {.bytes = "⡃"},
    {.bytes = "⡄"},
    {.bytes = "⡅"},
    {.bytes = "⡆"},
    {.bytes = "⡇"},
    {.bytes = "⡈"},
    {.bytes = "⡉"},
    {.bytes = "⡊"},
    {.bytes = "⡋"},
    {.bytes = "⡌"},
    {.bytes = "⡍"},
    {.bytes = "⡎"},
    {.bytes = "⡏"},
    {.bytes = "⡐"},
    {.bytes = "⡑"},
    {.bytes = "⡒"},
    {.bytes = "⡓"},
    {.bytes = "⡔"},
    {.bytes = "⡕"},
    {.bytes = "⡖"},
    {.bytes = "⡗"},
    {.bytes = "⡘"},
    {.bytes = "⡙"},
    {.bytes = "⡚"},
    {.bytes = "⡛"},
    {.bytes = "⡜"},
    {.bytes = "⡝"},
    {.bytes = "⡞"},
    {.bytes = "⡟"},
    {.bytes = "⡠"},
    {.bytes = "⡡"},
    {.bytes = "⡢"},
    {.bytes = "⡣"},
    {.bytes = "⡤"},
    {.bytes = "⡥"},
    {.bytes = "⡦"},
    {.bytes = "⡧"},
    {.bytes = "⡨"},
    {.bytes = "⡩"},
    {.bytes = "⡪"},
    {.bytes = "⡫"},
    {.bytes = "⡬"},
    {.bytes = "⡭"},
    {.bytes = "⡮"},
    {.bytes = "⡯"},
    {.bytes = "⡰"},
    {.bytes = "⡱"},
    {.bytes = "⡲"},
    {.bytes = "⡳"},
    {.bytes = "⡴"},
    {.bytes = "⡵"},
    {.bytes = "⡶"},
    {.bytes = "⡷"},
    {.bytes = "⡸"},
    {.bytes = "⡹"},
    {.bytes = "⡺"},
    {.bytes = "⡻"},
    {.bytes = "⡼"},
    {.bytes = "⡽"},
    {.bytes = "⡾"},
    {.bytes = "⡿"},
    {.bytes = "⢀"},
    {.bytes = "⢁"},
    {.bytes = "⢂"},
    {.bytes = "⢃"},
    {.bytes = "⢄"},
    {.bytes = "⢅"},
    {.bytes = "⢆"},
    {.bytes = "⢇"},
    {.bytes = "⢈"},
    {.bytes = "⢉"},
    {.bytes = "⢊"},
    {.bytes = "⢋"},
    {.bytes = "⢌"},
    {.bytes = "⢍"},
    {.bytes = "⢎"},
    {.bytes = "⢏"},
    {.bytes = "⢐"},
    {.bytes = "⢑"},
    {.bytes = "⢒"},
    {.bytes = "⢓"},
    {.bytes = "⢔"},
    {.bytes = "⢕"},
    {.bytes = "⢖"},
    {.bytes = "⢗"},
    {.bytes = "⢘"},
    {.bytes = "⢙"},
    {.bytes = "⢚"},
    {.bytes = "⢛"},
    {.bytes = "⢜"},
    {.bytes = "⢝"},
    {.bytes = "⢞"},
    {.bytes = "⢟"},
    {.bytes = "⢠"},
    {.bytes = "⢡"},
    {.bytes = "⢢"},
    {.bytes = "⢣"},
    {.bytes = "⢤"},
    {.bytes = "⢥"},
    {.bytes = "⢦"},
    {.bytes = "⢧"},
    {.bytes = "⢨"},
    {.bytes = "⢩"},
    {.bytes = "⢪"},
    {.bytes = "⢫"},
    {.bytes = "⢬"},
    {.bytes = "⢭"},
    {.bytes = "⢮"},
    {.bytes = "⢯"},
    {.bytes = "⢰"},
    {.bytes = "⢱"},
    {.bytes = "⢲"},
    {.bytes = "⢳"},
    {.bytes = "⢴"},
    {.bytes = "⢵"},
    {.bytes = "⢶"},
    {.bytes = "⢷"},
    {.bytes = "⢸"},
    {.bytes = "⢹"},
    {.bytes = "⢺"},
    {.bytes = "⢻"},
    {.bytes = "⢼"},
    {.bytes = "⢽"},
    {.bytes = "⢾"},
    {.bytes = "⢿"},
    {.bytes = "⣀"},
    {.bytes = "⣁"},
    {.bytes = "⣂"},
    {.bytes = "⣃"},
    {.bytes = "⣄"},
    {.bytes = "⣅"},
    {.bytes = "⣆"},
    {.bytes = "⣇"},
    {.bytes = "⣈"},
    {.bytes = "⣉"},
    {.bytes = "⣊"},
    {.bytes = "⣋"},
    {.bytes = "⣌"},
    {.bytes = "⣍"},
    {.bytes = "⣎"},
    {.bytes = "⣏"},
    {.bytes = "⣐"},
    {.bytes = "⣑"},
    {.bytes = "⣒"},
    {.bytes = "⣓"},
    {.bytes = "⣔"},
    {.bytes = "⣕"},
    {.bytes = "⣖"},
    {.bytes = "⣗"},
    {.bytes = "⣘"},
    {.bytes = "⣙"},
    {.bytes = "⣚"},
    {.bytes = "⣛"},
    {.bytes = "⣜"},
    {.bytes = "⣝"},
    {.bytes = "⣞"},
    {.bytes = "⣟"},
    {.bytes = "⣠"},
    {.bytes = "⣡"},
    {.bytes = "⣢"},
    {.bytes = "⣣"},
    {.bytes = "⣤"},
    {.bytes = "⣥"},
    {.bytes = "⣦"},
    {.bytes = "⣧"},
    {.bytes = "⣨"},
    {.bytes = "⣩"},
    {.bytes = "⣪"},
    {.bytes = "⣫"},
    {.bytes = "⣬"},
    {.bytes = "⣭"},
    {.bytes = "⣮"},
    {.bytes = "⣯"},
    {.bytes = "⣰"},
    {.bytes = "⣱"},
    {.bytes = "⣲"},
    {.bytes = "⣳"},
    {.bytes = "⣴"},
    {.bytes = "⣵"},
    {.bytes = "⣶"},
    {.bytes = "⣷"},
    {.bytes = "⣸"},
    {.bytes = "⣹"},
    {.bytes = "⣺"},
    {.bytes = "⣻"},
    {.bytes = "⣼"},
    {.bytes = "⣽"},
    {.bytes = "⣾"},
    {.bytes = "⣿"}
};

size_t braille_index(uint8_t x)
{
    const size_t group_size = 64;
    const size_t bgroup = ((x & 0b00001000) >> 3) | ((x & 0b10000000) >> 6);
    const size_t boffset = (x & 0b00000111) | ((x & 0b01110000) >> 1);
    return bgroup * group_size + boffset;
}

void pixels_to_braille_chars(Braille *output, uint32_t *input)
{
    for (int row = 0; row < CONSOLE_ROWS; ++row) {
        for (int col = 0; col < CONSOLE_COLS; ++col) {
            uint8_t braille_mask = 0;

            for (int dot_index = 0; dot_index < BRAILLE_DOTS_COUNT; ++dot_index) {
                const int dot_col = dot_index / BRAILLE_DOTS_ROWS;
                const int dot_row = dot_index % BRAILLE_DOTS_ROWS;
                uint32_t bright_sum = 0;
                for (int y = 0; y < BRAILLE_DOT_HEIGHT_IN_PIXELS; ++y) {
                    for (int x = 0; x < BRAILLE_DOT_WIDTH_IN_PIXELS; ++x) {
                        const int pixel_x =
                            col * CHAR_WIDTH_IN_PIXELS +
                            dot_col * BRAILLE_DOT_WIDTH_IN_PIXELS +
                            x;
                        const int pixel_y =
                            row * CHAR_HEIGHT_IN_PIXELS +
                            dot_row * BRAILLE_DOT_HEIGHT_IN_PIXELS +
                            y;
                        const int index = pixel_y * DISPLAY_WIDTH + pixel_x;
                        bright_sum += abgr_bright(input[index]);
                    }
                }

                bright_sum /= BRAILLE_DOT_WIDTH_IN_PIXELS * BRAILLE_DOT_HEIGHT_IN_PIXELS;
                assert(bright_sum <= UINT8_MAX);
                if (bright_sum >= 126) {
                    braille_mask |= (1 << dot_index);
                }
            }

            output[row * CONSOLE_COLS + col] = brailles[braille_index(braille_mask)];
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
    setlocale(LC_ALL, "");

    static Braille output[CONSOLE_ROWS * CONSOLE_COLS] = {0};

    init();

    const size_t display_width = get_display_width();
    const size_t display_height = get_display_height();
    assert(display_width == DISPLAY_WIDTH);
    assert(display_height == DISPLAY_HEIGHT);

    uint32_t *pixels = get_display();
    printf("Display %zux%zu at %p\n", display_width, display_height, pixels);

    size_t fps = 60;

    initscr();
    cbreak();
    noecho();
    timeout(1000 / fps);

    int32_t x = DISPLAY_WIDTH / 2;
    mouse_move(x, 0);

    while (1) {
        next_frame(1.0f / 60.0f);
        pixels_to_braille_chars(output, pixels);

        for (int row = 0; row < CONSOLE_ROWS; ++row) {
            mvaddnstr(row, 0, (char*) &output[row * CONSOLE_COLS], CONSOLE_COLS * sizeof(output[0]));
        }
        refresh();

        int code = getch();
        if (code == 'a') {
            x -= 20;
            mouse_move(x, 0);
        } else if (code == 'd') {
            x += 20;
            mouse_move(x, 0);
        } else if (code == ' ') {
            mouse_click(x, 0);
        } else if (code == 'p') {
            toggle_pause();
        }

        usleep(1000000 / fps);
    }

    return 0;
}

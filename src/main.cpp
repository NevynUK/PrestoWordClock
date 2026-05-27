#include "libraries/pico_graphics/pico_graphics.hpp"
#include "drivers/st7701/st7701.hpp"

#include "pico/multicore.h"
#include "pico/stdlib.h"

#include <cstring>

#include "word_clock.hpp"
#include "time_manager.hpp"

using namespace pimoroni;

// ─── Display constants ───────────────────────────────────────────────────────

#define FRAME_WIDTH  240
#define FRAME_HEIGHT 240

static const uint BACKLIGHT = 45;
static const uint LCD_CLK   = 26;
static const uint LCD_CS    = 28;
static const uint LCD_DAT   = 27;
static const uint LCD_DC    = static_cast<uint>(-1);
static const uint LCD_D0    = 1;

uint16_t back_buffer [FRAME_WIDTH * FRAME_HEIGHT];
uint16_t front_buffer[FRAME_WIDTH * FRAME_HEIGHT];

// ─── Layout constants ────────────────────────────────────────────────────────
//
// Character scale=3 → each glyph is 18 px wide × 24 px tall (6×8 base font).
// Cell size: 21 × 24 px  →  11 cols × 21 = 231 px,  10 rows × 24 = 240 px.
// A 4 px left margin centres the 231 px grid inside the 240 px display.

static constexpr int CHAR_SCALE  = 3;
static constexpr int CHAR_W      = 6 * CHAR_SCALE; // 18 px
static constexpr int CHAR_H      = 8 * CHAR_SCALE; // 24 px
static constexpr int CELL_W      = 21;
static constexpr int CELL_H      = 24;
static constexpr int GRID_X      = (FRAME_WIDTH  - GRID_COLS * CELL_W) / 2; // 4 px
static constexpr int GRID_Y      = (FRAME_HEIGHT - GRID_ROWS * CELL_H) / 2; // 0 px
static constexpr int CHAR_OFF_X  = (CELL_W - CHAR_W) / 2; // 1 px
static constexpr int CHAR_OFF_Y  = (CELL_H - CHAR_H) / 2; // 0 px

// ─── Colours (RGB565) ────────────────────────────────────────────────────────

static int PEN_BG;
static int PEN_DIM;
static int PEN_LIT;

// ─── Global display pointers (used by render_status callback) ────────────────

static ST7701*                 g_presto  = nullptr;
static PicoGraphics_PenRGB565* g_display = nullptr;

// ─── Status screen renderer ──────────────────────────────────────────────────
//
// msg is '\n'-delimited.  The first segment is rendered large (scale 3) in
// PEN_LIT; each subsequent segment is rendered smaller (scale 2) in PEN_DIM.

static void render_status(const char* msg) {
    if (!g_display || !g_presto) return;

    char buf[80];
    strncpy(buf, msg, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    g_display->set_pen(PEN_BG);
    g_display->clear();

    char* p  = buf;
    char* nl = strchr(p, '\n');
    if (nl) *nl++ = '\0';

    // Title line — large
    g_display->set_pen(PEN_LIT);
    g_display->text(p, {20, 70}, 200, 3);

    // Detail lines — small, stacked below title
    int y = 135;
    p = nl;
    while (p && *p) {
        nl = strchr(p, '\n');
        if (nl) *nl++ = '\0';
        g_display->set_pen(PEN_DIM);
        g_display->text(p, {10, y}, 220, 2);
        y += 30;
        p = nl;
    }

    g_presto->update(g_display);
}

// ─── Entry point ─────────────────────────────────────────────────────────────

int main() {
    set_sys_clock_khz(240000, true);
    stdio_init_all();

    // Deassert LCD chip-select before talking to it
    gpio_init(LCD_CS);
    gpio_put(LCD_CS, 1);
    gpio_set_dir(LCD_CS, true);

    // Initialise display
    ST7701 presto(FRAME_WIDTH, FRAME_HEIGHT, ROTATE_0,
                  SPIPins{spi1, LCD_CS, LCD_CLK, LCD_DAT, PIN_UNUSED, LCD_DC, BACKLIGHT},
                  back_buffer);
    PicoGraphics_PenRGB565 display(FRAME_WIDTH, FRAME_HEIGHT, front_buffer);

    presto.init();

    // Set up colour pens
    PEN_BG  = display.create_pen(  5,   5,  20);
    PEN_DIM = display.create_pen( 55,  50,  40);
    PEN_LIT = display.create_pen(255, 200,  50);

    // Expose display to the status callback
    g_presto  = &presto;
    g_display = &display;

    // Connect to WiFi and sync time; render_status updates the display on each step
    time_manager_init(render_status);

    bool lit[GRID_ROWS][GRID_COLS];
    int  prev_min5 = -1;

    while (true) {
        int hour, minute, second;
        time_manager_get(hour, minute, second);

        if (!time_manager_is_synced()) {
            display.set_pen(PEN_BG);
            display.clear();
            display.set_pen(PEN_DIM);
            display.text("NO SYNC", {30, 108}, 240, 3);
            presto.update(&display);
            sleep_ms(500);
            continue;
        }

        int min5 = (minute + 2) / 5 * 5;
        if (min5 == prev_min5) {
            sleep_ms(200);
            continue;
        }
        prev_min5 = min5;

        get_lit_cells(hour, minute, lit);

        display.set_pen(PEN_BG);
        display.clear();

        for (int row = 0; row < GRID_ROWS; ++row) {
            for (int col = 0; col < GRID_COLS; ++col) {
                int x = GRID_X + col * CELL_W + CHAR_OFF_X;
                int y = GRID_Y + row * CELL_H + CHAR_OFF_Y;
                display.set_pen(lit[row][col] ? PEN_LIT : PEN_DIM);
                display.character(GRID[row][col], {x, y}, CHAR_SCALE);
            }
        }

        presto.update(&display);
    }
}
#pragma once

#include <cstring>

static constexpr int GRID_ROWS = 10;
static constexpr int GRID_COLS = 11;

// The 11×10 letter grid. Each row is a null-terminated 11-character string.
extern const char GRID[GRID_ROWS][GRID_COLS + 1];

// Fills lit[row][col] = true for every cell that should be illuminated for
// the given 24-hour time.
void get_lit_cells(int hour, int minute, bool lit[GRID_ROWS][GRID_COLS]);

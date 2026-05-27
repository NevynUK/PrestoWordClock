/**
 * @file word_clock.hpp
 * @brief Letter grid layout and cell-lighting API for the Word Clock.
 *
 * Defines the 12×11 character grid and declares get_lit_cells(), which maps a
 * wall-clock time to the set of grid cells that should be illuminated.
 */
#pragma once

#include <cstring>

/// Number of rows in the letter grid.
static constexpr int GRID_ROWS = 11;

/// Number of columns in the letter grid.
static constexpr int GRID_COLS = 12;

/// The 12×11 letter grid; each element is a null-terminated 12-character string.
extern const char GRID[GRID_ROWS][GRID_COLS + 1];

/**
 * @brief Populates a 2-D boolean array with the cells that should be lit for
 *        the given time.
 *
 * Always lights IT and IS.  Lights NEARLY when the minute is 3 or 4 past a
 * 5-minute boundary; lights JUST and AFTER when it is 1 or 2 minutes past.
 * The minute phrase (FIVE, TEN, QUARTER, …) and hour word are chosen for the
 * nearest displayed time.
 *
 * @param hour    Current hour in 24-hour format (0–23).
 * @param minute  Current minute (0–59).
 * @param lit     Output array [GRID_ROWS][GRID_COLS].  Each element is set to
 *                @c true if the cell should be illuminated, @c false otherwise.
 */
void get_lit_cells(int hour, int minute, bool lit[GRID_ROWS][GRID_COLS]);

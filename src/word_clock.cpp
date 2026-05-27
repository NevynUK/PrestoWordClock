/**
 * @file word_clock.cpp
 * @brief Letter grid definition and time-to-cell mapping implementation.
 *
 * Contains the GRID character array and the two internal helpers (light,
 * light_hour) used by get_lit_cells() to illuminate the correct words for
 * a given wall-clock time.
 */
#include "word_clock.hpp"

// ─── Grid ────────────────────────────────────────────────────────────────────
//
//  Row  String          Words hidden inside
//  ---  ------------    --------------------------------------------------------
//   0   ITHISUNEARLY    IT(0-1)  IS(3-4)  NEARLY(6-11)  +HIS(2-4)  +SUN(4-6)
//   1   JUSTEAFTERRY    JUST(0-3)  AFTER(5-9)  +TEA(3-5)  +TERRY(7-11)
//   2   ATQUARTERSLY    QUARTER(2-8)  +AT(0-1)  +QUARTERS(2-9)  +SLY(9-11)
//   3   TWENTYOFIVES    TWENTY(0-5)  FIVE-min(7-10)  +YO(5-6)  +OF(6-7)  +FIVES(7-11)
//   4   HALFATENOTON    HALF(0-3)  TEN-min(5-7)  TO(9-10)  +FAT(3-5)  +NOT(7-9)  +ON(10-11)
//   5   PASTERUNINES    PAST(0-3)  NINE(7-10)  +PASTE(0-4)  +RUN(5-7)  +NINES(7-11)
//   6   ONESIXTHREEL    ONE(0-2)  SIX(3-5)  THREE(6-10)  +EEL(9-11)
//   7   FOURFIVETWOE    FOUR(0-3)  FIVE-hr(4-7)  TWO(8-10)  +WOE(9-11)  +OE(10-11)
//   8   EIGHTELEVENS    EIGHT(0-4)  ELEVEN(5-10)  +ELEVENS(5-11)
//   9   SEVENTWELVET    SEVEN(0-4)  TWELVE(5-10)  +VET(9-11)
//  10   TENATOCLOCKS    TEN-hr(0-2)  OCLOCK(5-10)  +AT(3-4)  +TO(4-5)  +CLOCKS(6-11)

const char GRID[GRID_ROWS][GRID_COLS + 1] = {
    "ITHISUNEARLY", "JUSTEAFTERRY", "ATQUARTERSLY", "TWENTYOFIVES", "HALFATENOTON", "PASTERUNINES", "ONESIXTHREEL", "FOURFIVETWOE", "EIGHTELEVENS", "SEVENTWELVET", "TENATOCLOCKS",
};

// ─── Helpers ─────────────────────────────────────────────────────────────────

/**
 * @brief Lights a contiguous horizontal run of cells in the grid.
 *
 * @param lit  Grid state array to modify.
 * @param row  Zero-based row index.
 * @param col  Zero-based starting column.
 * @param len  Number of consecutive columns to light.
 */
static void light(bool lit[GRID_ROWS][GRID_COLS], int row, int col, int len)
{
    for (int i = 0; i < len; ++i)
    {
        lit[row][col + i] = true;
    }
}

/**
 * @brief Lights the hour word for a given hour value.
 *
 * Applies @c h % 12 so that both 0 and 12 map to TWELVE.
 *
 * @param lit  Grid state array to modify.
 * @param h    Hour value (0–23); modulo 12 is applied internally.
 */
static void light_hour(bool lit[GRID_ROWS][GRID_COLS], int h)
{
    switch (h % 12)
    {
        case 0:
            light(lit, 9, 5, 6);
            break; // TWELVE
        case 1:
            light(lit, 6, 0, 3);
            break; // ONE
        case 2:
            light(lit, 7, 8, 3);
            break; // TWO
        case 3:
            light(lit, 6, 6, 5);
            break; // THREE
        case 4:
            light(lit, 7, 0, 4);
            break; // FOUR
        case 5:
            light(lit, 7, 4, 4);
            break; // FIVE
        case 6:
            light(lit, 6, 3, 3);
            break; // SIX
        case 7:
            light(lit, 9, 0, 5);
            break; // SEVEN
        case 8:
            light(lit, 8, 0, 5);
            break; // EIGHT
        case 9:
            light(lit, 5, 7, 4);
            break; // NINE
        case 10:
            light(lit, 10, 0, 3);
            break; // TEN
        case 11:
            light(lit, 8, 5, 6);
            break; // ELEVEN
    }
}

void get_lit_cells(int hour, int minute, bool lit[GRID_ROWS][GRID_COLS])
{
    memset(lit, 0, sizeof(bool) * GRID_ROWS * GRID_COLS);

    // IT IS — always on
    light(lit, 0, 0, 2); // IT
    light(lit, 0, 3, 2); // IS

    // Determine modifier: offset 1-2 = JUST AFTER, offset 3-4 = NEARLY
    int offset = minute % 5;
    int bucket = (minute / 5) * 5;

    if (offset >= 3)
    {
        light(lit, 0, 6, 6); // NEARLY
    }
    else if (offset >= 1)
    {
        light(lit, 1, 0, 4); // JUST
        light(lit, 1, 5, 5); // AFTER
    }

    // For "nearly", show the next phrase; otherwise show the current bucket.
    // Check min5 > 30 before wrapping 60→0 so that :58/:59 correctly
    // advances the hour to display (e.g. "nearly twelve o'clock").
    int min5 = (offset >= 3) ? bucket + 5 : bucket;

    int h = hour;
    if (min5 > 30)
    {
        h += 1;
    }

    if (min5 == 60)
    {
        min5 = 0;
    }

    switch (min5)
    {
        case 0:
            light(lit, 10, 5, 6); // OCLOCK
            break;
        case 5:
            light(lit, 3, 7, 4);  // FIVE
            light(lit, 5, 0, 4);  // PAST
            break;
        case 10:
            light(lit, 4, 5, 3);  // TEN
            light(lit, 5, 0, 4);  // PAST
            break;
        case 15:
            light(lit, 2, 2, 7);  // QUARTER
            light(lit, 5, 0, 4);  // PAST
            break;
        case 20:
            light(lit, 3, 0, 6);  // TWENTY
            light(lit, 5, 0, 4);  // PAST
            break;
        case 25:
            light(lit, 3, 0, 6);  // TWENTY
            light(lit, 3, 7, 4);  // FIVE
            light(lit, 5, 0, 4);  // PAST
            break;
        case 30:
            light(lit, 4, 0, 4);  // HALF
            light(lit, 5, 0, 4);  // PAST
            break;
        case 35:
            light(lit, 3, 0, 6);  // TWENTY
            light(lit, 3, 7, 4);  // FIVE
            light(lit, 4, 9, 2);  // TO
            break;
        case 40:
            light(lit, 3, 0, 6);  // TWENTY
            light(lit, 4, 9, 2);  // TO
            break;
        case 45:
            light(lit, 2, 2, 7);  // QUARTER
            light(lit, 4, 9, 2);  // TO
            break;
        case 50:
            light(lit, 4, 5, 3);  // TEN
            light(lit, 4, 9, 2);  // TO
            break;
        case 55:
            light(lit, 3, 7, 4);  // FIVE
            light(lit, 4, 9, 2);  // TO
            break;
    }

    light_hour(lit, h);
}

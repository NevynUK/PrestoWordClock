#include "word_clock.hpp"

// ─── Grid ────────────────────────────────────────────────────────────────────
//
//  Row  String        Words hidden inside
//  ---  -----------   --------------------------------
//   0   ITLISASAMPM   IT(0-1)  IS(3-4)
//   1   ACQUARTERDC   QUARTER(2-8)
//   2   TWENTYXFIVE   TWENTY(0-5)  FIVE-min(7-10)
//   3   HALFSTENFTO   HALF(0-3)  TEN-min(5-7)  TO(9-10)
//   4   PASTERUNINE   PAST(0-3)  NINE(7-10)
//   5   ONESIXTHREE   ONE(0-2)  SIX(3-5)  THREE(6-10)
//   6   FOURFIVETWO   FOUR(0-3)  FIVE-hr(4-7)  TWO(8-10)
//   7   EIGHTELEVEN   EIGHT(0-4)  ELEVEN(5-10)
//   8   SEVENTWELVE   SEVEN(0-4)  TWELVE(5-10)
//   9   TENSEOCLOCK   TEN-hr(0-2)  OCLOCK(5-10)

const char GRID[GRID_ROWS][GRID_COLS + 1] = {
    "ITLISASAMPM", "ACQUARTERDC", "TWENTYXFIVE", "HALFSTENFTO", "PASTERUNINE", "ONESIXTHREE", "FOURFIVETWO", "EIGHTELEVEN", "SEVENTWELVE", "TENSEOCLOCK",
};

// ─── Helpers ─────────────────────────────────────────────────────────────────

static void light(bool lit[GRID_ROWS][GRID_COLS], int row, int col, int len)
{
    for (int i = 0; i < len; ++i)
        lit[row][col + i] = true;
}

static void light_hour(bool lit[GRID_ROWS][GRID_COLS], int h)
{
    switch (h % 12)
    {
        case 0:
            light(lit, 8, 5, 6);
            break; // TWELVE
        case 1:
            light(lit, 5, 0, 3);
            break; // ONE
        case 2:
            light(lit, 6, 8, 3);
            break; // TWO
        case 3:
            light(lit, 5, 6, 5);
            break; // THREE
        case 4:
            light(lit, 6, 0, 4);
            break; // FOUR
        case 5:
            light(lit, 6, 4, 4);
            break; // FIVE
        case 6:
            light(lit, 5, 3, 3);
            break; // SIX
        case 7:
            light(lit, 8, 0, 5);
            break; // SEVEN
        case 8:
            light(lit, 7, 0, 5);
            break; // EIGHT
        case 9:
            light(lit, 4, 7, 4);
            break; // NINE
        case 10:
            light(lit, 9, 0, 3);
            break; // TEN
        case 11:
            light(lit, 7, 5, 6);
            break; // ELEVEN
    }
}

// ─── Public API ──────────────────────────────────────────────────────────────

void get_lit_cells(int hour, int minute, bool lit[GRID_ROWS][GRID_COLS])
{
    memset(lit, 0, sizeof(bool) * GRID_ROWS * GRID_COLS);

    // IT IS — always on
    light(lit, 0, 0, 2); // IT
    light(lit, 0, 3, 2); // IS

    // Round to the nearest 5 minutes
    int min5 = (minute + 2) / 5 * 5;

    // For "X TO", the hour advances by one.
    // Check before resetting 60→0 so that minutes 57-59 (which round up to
    // the next hour) correctly advance the displayed hour.
    int h = hour;
    if (min5 > 30)
        h += 1;

    if (min5 == 60)
        min5 = 0;

    switch (min5)
    {
        case 0:
            light(lit, 9, 5, 6); // OCLOCK
            break;
        case 5:
            light(lit, 2, 7, 4); // FIVE
            light(lit, 4, 0, 4); // PAST
            break;
        case 10:
            light(lit, 3, 5, 3); // TEN
            light(lit, 4, 0, 4); // PAST
            break;
        case 15:
            light(lit, 1, 2, 7); // QUARTER
            light(lit, 4, 0, 4); // PAST
            break;
        case 20:
            light(lit, 2, 0, 6); // TWENTY
            light(lit, 4, 0, 4); // PAST
            break;
        case 25:
            light(lit, 2, 0, 6); // TWENTY
            light(lit, 2, 7, 4); // FIVE
            light(lit, 4, 0, 4); // PAST
            break;
        case 30:
            light(lit, 3, 0, 4); // HALF
            light(lit, 4, 0, 4); // PAST
            break;
        case 35:
            light(lit, 2, 0, 6); // TWENTY
            light(lit, 2, 7, 4); // FIVE
            light(lit, 3, 9, 2); // TO
            break;
        case 40:
            light(lit, 2, 0, 6); // TWENTY
            light(lit, 3, 9, 2); // TO
            break;
        case 45:
            light(lit, 1, 2, 7); // QUARTER
            light(lit, 3, 9, 2); // TO
            break;
        case 50:
            light(lit, 3, 5, 3); // TEN
            light(lit, 3, 9, 2); // TO
            break;
        case 55:
            light(lit, 2, 7, 4); // FIVE
            light(lit, 3, 9, 2); // TO
            break;
    }

    light_hour(lit, h);
}

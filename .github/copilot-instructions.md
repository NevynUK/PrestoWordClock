# Copilot Instructions — PrestoWordClock

## Project Overview

A **Word Clock** firmware for the [Pimoroni Presto](https://shop.pimoroni.com/products/presto) development board (RP2350 / Raspberry Pi 2350 microcontroller). Written in **C++17**, built with **CMake** and the **Raspberry Pi Pico SDK 2.2.0**.

The clock displays the current time as English words on the Presto's 240×240 px ST7701 LCD. Active letters are rendered in warm amber; inactive letters are dimmed. Time is synchronised over WiFi using NTP on boot and resynced every hour automatically.

---

## Repository Structure

```
WordClock/
├── src/
│   ├── main.cpp            # Display init, status screen, main render loop
│   ├── word_clock.cpp/hpp  # Maps hour + minute → set of lit grid cells
│   ├── time_manager.cpp/hpp# WiFi + NTP time sync; elapsed-time tracking
│   ├── wifi.h              # WiFi credentials and UTC offset — GITIGNORED, never commit
│   ├── wifi.h.example      # Safe template — copy to wifi.h and fill in credentials
│   └── lwipopts.h          # lwIP configuration (UDP/DHCP/DNS/SNTP only)
├── CMakeLists.txt          # Board detection, WiFi libraries, build targets
├── build.sh                # cmake configure + make; auto-clones dependencies
├── flash.sh                # OpenOCD flash via Raspberry Pi Debug Probe
├── debug.sh                # OpenOCD + arm-none-eabi-gdb debug session
├── run-checks.sh           # Runs cppcheck and clang-format checks
├── .clang-format           # Code style: Microsoft-based, C++17, 4-space indent
├── pico_sdk_import.cmake   # Pico SDK locator (reads PICO_SDK_PATH env var)
└── pimoroni_pico_import.cmake # pimoroni-pico library locator
```

---

## Build Dependencies

The following are **not** included in the repo and must be present locally (or are cloned automatically by `build.sh`):

| Dependency | Repo | Notes |
|---|---|---|
| Pico SDK | `raspberrypi/pico-sdk` @ `2.2.0` | Set `PICO_SDK_PATH` env var |
| pimoroni-pico | `pimoroni/pimoroni-pico` | Graphics/display drivers; `build.sh` auto-clones to `../pimoroni-pico` |
| presto | `pimoroni/presto` | Board header (`presto.h`) with CYW43 pin defs; auto-cloned to `../presto` |

### Local Build

```bash
./build.sh           # configure + build (auto-clones pimoroni-pico and presto if needed)
./build.sh --clean   # wipe build dir first
```

The script searches for `pimoroni-pico` and `presto` in sibling directories before cloning.
`PICO_SDK_PATH` must be set in the environment, or the SDK must be at the VS Code extension default location (`~/.pico-sdk`).

### WiFi Credentials

Copy `src/wifi.h.example` → `src/wifi.h` and set:

```cpp
#define WIFI_SSID     "your-network"
#define WIFI_PASSWORD "your-password"
#define UTC_OFFSET_SECONDS 3600   // e.g. 3600 for BST, 0 for UTC
```

`src/wifi.h` is gitignored. **Never commit it.**

---

## Word Grid (11 × 10)

```
I T L I S A S A M P M   ← IT IS
A C Q U A R T E R D C   ← QUARTER
T W E N T Y X F I V E   ← TWENTY FIVE
H A L F S T E N F T O   ← HALF TEN TO
P A S T E R U N I N E   ← PAST NINE
O N E S I X T H R E E   ← ONE SIX THREE
F O U R F I V E T W O   ← FOUR FIVE TWO
E I G H T E L E V E N   ← EIGHT ELEVEN
S E V E N T W E L V E   ← SEVEN TWELVE
T E N S E O C L O C K   ← TEN O'CLOCK
```

Minutes are rounded to the nearest 5: O'CLOCK → FIVE PAST → TEN PAST → QUARTER PAST → TWENTY PAST → TWENTY FIVE PAST → HALF PAST → TWENTY FIVE TO → TWENTY TO → QUARTER TO → TEN TO → FIVE TO.

---

## Display Layout

- Scale-3 characters: 18×24 px glyph, 21×24 px cell.
- 11 columns × 21 px = 231 px; 4 px left margin centres the grid in 240 px.
- 10 rows × 24 px = 240 px (exact fit, no vertical margin needed).
- Background: dark navy `#050514`; inactive letters: dim warm grey; active letters: warm amber `#FFC832`.

---

## Key Implementation Notes

- `PICO_BOARD=presto` and `PICO_BOARD_HEADER_DIRS` (pointing to `presto/boards/presto/`) must be set **before** `include(pico_sdk_import.cmake)` in `CMakeLists.txt`. `PICO_CYW43_SUPPORTED=1` must also be set at CMake level to enable the CYW43 WiFi driver.
- The `SNTP_SET_SYSTEM_TIME` macro in lwIP defaults to a no-op. It is overridden in `lwipopts.h` to invoke `sntp_set_system_time()` defined in `time_manager.cpp`. The forward declaration is guarded with `#ifndef __cplusplus` to avoid a conflicting-declaration error.
- lwIP passes **Unix time** (seconds since 1970) to `SNTP_SET_SYSTEM_TIME` — the NTP→Unix epoch conversion is handled internally via `DIFF_SEC_1970_2036`. No further subtraction is needed in application code.
- `MEMP_NUM_SYS_TIMEOUT` must be at least 17; the default is too small for the CYW43 driver + DHCP + DNS + SNTP timer registrations running concurrently.
- NTP sync cycles through `0–3.pool.ntp.org`, 5 times each (20 attempts), waiting up to 4 seconds per attempt. SNTP resyncs automatically every hour in the background.

---

## Code Style

Enforced by `.clang-format` (based on Microsoft style, C++17):

- 4-space indentation, no tabs.
- `ColumnLimit: 300` (long lines permitted).
- Braces always on new line (`BreakBeforeBraces: Custom`).
- `SortIncludes: Never`.

Run checks locally with:
```bash
./run-checks.sh           # cppcheck + clang-format dry-run
./run-checks.sh --verbose # verbose output
```

---

## CI / GitHub Actions

Two workflows run on every push to `main` and on every pull request:

| Workflow | File | What it does |
|---|---|---|
| **Build** | `.github/workflows/build.yml` | Checks out pico-sdk 2.2.0, pimoroni-pico, presto; installs `arm-none-eabi-gcc 13.3.Rel1` via `carlosperate/arm-none-eabi-gcc-action@v1`; builds with ccache; uploads `word-clock.uf2` as an artifact |
| **Run Checks** | `.github/workflows/checks.yml` | Installs `cppcheck` and `clang-format`; runs `./run-checks.sh` |

Branch protection on `main` requires both checks to pass before a PR can be merged (`strict: true` — the branch must also be up to date with `main`).

The CI build uses `cmake` directly (not `build.sh`) to allow explicit path control and ccache integration.

---

## Hardware

- **Board**: [Pimoroni Presto](https://shop.pimoroni.com/products/presto) — RP2350, CYW43 WiFi, 240×240 ST7701 LCD.
- **Debugger**: [Raspberry Pi Debug Probe](https://shop.pimoroni.com/products/raspberry-pi-debug-probe) — SWD + UART via OpenOCD.
- Flash firmware: `./flash.sh`
- Start debug session: `./debug.sh`

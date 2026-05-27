#!/usr/bin/env bash
# Flash the Word Clock firmware using the Raspberry Pi Debug Probe (CMSIS-DAP).
# Usage: ./flash.sh
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ELF="${SCRIPT_DIR}/build/word-clock.elf"

if [[ ! -f "${ELF}" ]]; then
    echo "Error: ${ELF} not found — run ./build.sh first."
    exit 1
fi

# Prefer the SDK-managed OpenOCD; fall back to whatever is on PATH.
OPENOCD_HOME="${HOME}/.pico-sdk/openocd/0.12.0+dev"
if [[ -x "${OPENOCD_HOME}/openocd" ]]; then
    OPENOCD="${OPENOCD_HOME}/openocd"
    OPENOCD_SCRIPTS="${OPENOCD_HOME}/scripts"
else
    OPENOCD="openocd"
    OPENOCD_SCRIPTS=""
fi

SCRIPTS_ARG=""
[[ -n "${OPENOCD_SCRIPTS}" ]] && SCRIPTS_ARG="-s ${OPENOCD_SCRIPTS}"

echo "==> Flashing ${ELF}"
# shellcheck disable=SC2086
"${OPENOCD}" ${SCRIPTS_ARG} \
    -f interface/cmsis-dap.cfg \
    -f target/rp2350.cfg \
    -c "adapter speed 5000" \
    -c "program ${ELF} verify reset exit"

echo "==> Flash complete"

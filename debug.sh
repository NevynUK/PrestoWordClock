#!/usr/bin/env bash
# Debug the Word Clock firmware via the Raspberry Pi Debug Probe.
# Opens an OpenOCD server then launches arm-none-eabi-gdb.
# Usage: ./debug.sh
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ELF="${SCRIPT_DIR}/build/word-clock.elf"

if [[ ! -f "${ELF}" ]]; then
    echo "Error: ${ELF} not found — run ./build.sh first."
    exit 1
fi

SDK_ROOT="${HOME}/.pico-sdk"
OPENOCD_HOME="${SDK_ROOT}/openocd/0.12.0+dev"
TOOLCHAIN_HOME="${SDK_ROOT}/toolchain/14_2_Rel1/bin"

if [[ -x "${OPENOCD_HOME}/openocd" ]]; then
    OPENOCD="${OPENOCD_HOME}/openocd"
    OPENOCD_SCRIPTS="${OPENOCD_HOME}/scripts"
else
    OPENOCD="openocd"
    OPENOCD_SCRIPTS=""
fi

if [[ -x "${TOOLCHAIN_HOME}/arm-none-eabi-gdb" ]]; then
    GDB="${TOOLCHAIN_HOME}/arm-none-eabi-gdb"
else
    GDB="arm-none-eabi-gdb"
fi

SCRIPTS_ARG=""
[[ -n "${OPENOCD_SCRIPTS}" ]] && SCRIPTS_ARG="-s ${OPENOCD_SCRIPTS}"

echo "==> Starting OpenOCD (CMSIS-DAP / Debug Probe)"
# shellcheck disable=SC2086
"${OPENOCD}" ${SCRIPTS_ARG} \
    -f interface/cmsis-dap.cfg \
    -f target/rp2350.cfg \
    -c "adapter speed 5000" &
OPENOCD_PID=$!

# Kill OpenOCD when this script exits for any reason
trap 'echo "==> Stopping OpenOCD"; kill "${OPENOCD_PID}" 2>/dev/null' EXIT

echo "==> Waiting for OpenOCD to start..."
sleep 2

echo "==> Launching GDB — type 'q' to quit"
"${GDB}" "${ELF}" \
    -ex "target extended-remote localhost:3333" \
    -ex "monitor reset init" \
    -ex "load" \
    -ex "monitor reset halt"

#!/usr/bin/env bash
# Build the Word Clock firmware.
# Usage: ./build.sh [--clean]
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

# ── Locate pimoroni-pico ──────────────────────────────────────────────────────
# Search in order of likelihood; update PIMORONI_PICO_SEARCH_PATHS to add more.
PIMORONI_PICO_SEARCH_PATHS=(
    "${SCRIPT_DIR}/../pimoroni-pico"          # sibling of project (auto-clone target)
    "${SCRIPT_DIR}/../../pimoroni-pico"       # one level above project
    "${HOME}/pimoroni-pico"                   # home directory clone
    "${HOME}/Developer/pimoroni-pico"
    "${HOME}/src/pimoroni-pico"
    "${HOME}/repos/pimoroni-pico"
)
PIMORONI_PICO_PATH=""
for p in "${PIMORONI_PICO_SEARCH_PATHS[@]}"; do
    if [[ -f "${p}/pimoroni_pico_import.cmake" ]]; then
        PIMORONI_PICO_PATH="$(cd "${p}" && pwd)"
        break
    fi
done
if [[ -z "${PIMORONI_PICO_PATH}" ]]; then
    PIMORONI_PICO_PATH="${SCRIPT_DIR}/../pimoroni-pico"
    echo "==> Cloning pimoroni-pico into ${PIMORONI_PICO_PATH}"
    git clone --depth 1 https://github.com/pimoroni/pimoroni-pico "${PIMORONI_PICO_PATH}"
fi
echo "==> pimoroni-pico: ${PIMORONI_PICO_PATH}"

# ── Locate pimoroni/presto ────────────────────────────────────────────────────
PIMORONI_PRESTO_SEARCH_PATHS=(
    "${SCRIPT_DIR}/../presto"
    "${SCRIPT_DIR}/../../presto"
    "${HOME}/presto"
    "${HOME}/Developer/presto"
    "${HOME}/src/presto"
    "${HOME}/repos/presto"
)
PIMORONI_PRESTO_PATH=""
for p in "${PIMORONI_PRESTO_SEARCH_PATHS[@]}"; do
    if [[ -d "${p}/drivers/st7701" ]]; then
        PIMORONI_PRESTO_PATH="$(cd "${p}" && pwd)"
        break
    fi
done
if [[ -z "${PIMORONI_PRESTO_PATH}" ]]; then
    PIMORONI_PRESTO_PATH="${SCRIPT_DIR}/../presto"
    echo "==> Cloning presto into ${PIMORONI_PRESTO_PATH}"
    git clone --depth 1 https://github.com/pimoroni/presto "${PIMORONI_PRESTO_PATH}"
fi
echo "==> presto:        ${PIMORONI_PRESTO_PATH}"

# ── Clean if requested or cache is stale ─────────────────────────────────────
if [[ "${1:-}" == "--clean" ]]; then
    echo "==> Cleaning build directory"
    rm -rf "${BUILD_DIR}"
fi

mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

if [[ -f CMakeCache.txt ]]; then
    CACHED_SRC=$(grep "^CMAKE_HOME_DIRECTORY" CMakeCache.txt 2>/dev/null | cut -d= -f2)
    if [[ "${CACHED_SRC}" != "${SCRIPT_DIR}" ]]; then
        echo "==> Stale CMake cache detected — clearing build directory"
        rm -rf ./*
    fi
fi

# ── Configure & build ─────────────────────────────────────────────────────────
echo "==> Configuring with CMake"
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DPIMORONI_PICO_PATH="${PIMORONI_PICO_PATH}" \
    -DPIMORONI_PRESTO_PATH="${PIMORONI_PRESTO_PATH}"

echo "==> Building"
CORES=$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)
make -j"${CORES}"

echo ""
echo "==> Done: ${BUILD_DIR}/word-clock.uf2"

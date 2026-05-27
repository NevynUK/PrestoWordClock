# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.12)

set(PICO_PLATFORM rp2350-arm-s)

# Use the Presto board header (provides CYW43 WiFi pin definitions).
# PIMORONI_PRESTO_PATH is passed in from build.sh via -DPIMORONI_PRESTO_PATH=...
if(DEFINED PIMORONI_PRESTO_PATH AND EXISTS "${PIMORONI_PRESTO_PATH}/boards/presto/presto.h")
    set(PICO_BOARD_HEADER_DIRS "${PIMORONI_PRESTO_PATH}/boards/presto")
    set(PICO_BOARD "presto")
    # Tell the Pico SDK to set up the CYW43 WiFi library targets.
    # The presto.h board header defines the CYW43 pin assignments but does not
    # set this cmake flag, so we must do it explicitly.
    set(PICO_CYW43_SUPPORTED 1)
    message("Board: presto (WiFi enabled)")
else()
    set(PICO_BOARD "pimoroni_pico_plus2_rp2350")
    message("WARNING: Presto board header not found — WiFi disabled")
endif()

set(NAME word-clock)

include(pimoroni_pico_import.cmake)
include(pico_sdk_import.cmake)

# Gooey boilerplate
project(${NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

if (NOT PIMORONI_PRESTO_PATH)
    set(PIMORONI_PRESTO_PATH ../../presto/)
endif()
if(NOT IS_ABSOLUTE ${PIMORONI_PRESTO_PATH})
    get_filename_component(
        PIMORONI_PRESTO_PATH
        "${CMAKE_CURRENT_BINARY_DIR}/${PIMORONI_PRESTO_PATH}"
        ABSOLUTE)
endif()
message("PIMORONI_PRESTO_PATH is ${PIMORONI_PRESTO_PATH}")
set(PIMORONI_PRESTO_PATH ${PIMORONI_PRESTO_PATH} CACHE PATH "Path to the Presto libraries" FORCE)
include_directories(${PIMORONI_PRESTO_PATH})
list(APPEND CMAKE_MODULE_PATH ${PIMORONI_PRESTO_PATH})
include(drivers/st7701/st7701_presto)

# Add your source files
add_executable(${NAME}
    src/main.cpp
    src/word_clock.cpp
    src/time_manager.cpp
)

# Make src/ visible on the include path so lwipopts.h and wifi.h are found
target_include_directories(${NAME} PRIVATE src)

# Include required libraries
# This assumes `pimoroni-pico` is stored alongside your project
include(common/pimoroni_i2c)
include(common/pimoroni_bus)
include(libraries/pico_graphics/pico_graphics)

# Don't forget to link the libraries you need!
target_link_libraries(${NAME}
    st7701_presto
    pico_stdlib
    pico_multicore
    pimoroni_i2c
    hardware_interp
    pico_graphics
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_sntp
)

target_compile_definitions(${NAME} PRIVATE
  PICO_CLOCK_AJDUST_PERI_CLOCK_WITH_SYS_CLOCK=1
)

# create map/bin/hex file etc.
pico_add_extra_outputs(${NAME})

# Enable USB UART output only
pico_enable_stdio_uart(${NAME} 0)
pico_enable_stdio_usb(${NAME} 1)

# Set up files for the release packages
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.uf2
    ${CMAKE_CURRENT_LIST_DIR}/README.md
    DESTINATION .
)

set(CPACK_PACKAGE_FILE_NAME ${NAME})
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_GENERATOR "ZIP" "TGZ")
include(CPack)
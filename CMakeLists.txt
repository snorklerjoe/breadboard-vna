cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(SuperVNA)

pico_sdk_init()
add_executable(SuperVNA)
pico_generate_pio_header(SuperVNA ${CMAKE_CURRENT_LIST_DIR}/blinky.pio)

target_sources(SuperVNA PRIVATE
    main.c
)

target_link_libraries(SuperVNA PRIVATE
    pico2_stdlib
    hardware_adc
    hardware_gpio
    hardware_pio
    hardware_irq
    pico2_bootsel_via_double_reset
)

pico_enable_stdio_usb(SuperVNA 1)
pico_add_extra_outputs(SuperVNA)
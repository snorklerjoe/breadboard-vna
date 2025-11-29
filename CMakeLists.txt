cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(SuperVNA)

pico_sdk_init()
add_executable(SuperVNA)
pico_generate_pio_header(SuperVNA ${CMAKE_CURRENT_LIST_DIR}/square.pio)
pico_generate_pio_header(SuperVNA ${CMAKE_CURRENT_LIST_DIR}/losquare.pio)
pico_generate_pio_header(SuperVNA ${CMAKE_CURRENT_LIST_DIR}/SPIPIO.pio)

target_sources(SuperVNA PRIVATE
    main.c
    pio.h
    pio.c
    ad9834.c
    receiver.c
    adc_sampling.c
    vna.c
)

target_link_libraries(SuperVNA PRIVATE
    pico_stdlib
    hardware_adc
    hardware_gpio
    hardware_pio
    hardware_irq
    hardware_dma
    pico_bootsel_via_double_reset
    hardware_spi
)

pico_enable_stdio_usb(SuperVNA 1)
pico_add_extra_outputs(SuperVNA)
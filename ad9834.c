#include "ad9834.h"
#include <stdint.h>
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"

void ad9834_init() {
    // Set pin modes & functions
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);

    // Init spi functionality
    spi_init(spi_default, 1000 * 1000);
    spi_set_format(spi_default, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
}

void ad9834_setfreq(unsigned int freq) {
    uint16_t src[] = {0b0010000000000000,
                      0b0100000000000000,
                      0b0100000000111111,
                      0x38};
    spi_write16_blocking(spi_default, &src[0], 4);
    // TODO: Set ad9834 freq to given value in kHz
}

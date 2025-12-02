/* Simple library for communicating with the ad9834 board
*/

#include "ad9834.h"
#include <stdint.h>
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include <hardware/clocks.h>
#include "pio.h"

// Control Register State
//   + Enables 2-word freq writes
//   + Disables freq/phase register switching using FSEL
const static uint16_t _init_code = 0x2000;

// Square wave reference for the AD9834
const static long _ad9834_clock_freq = 75000000;
const static double _freq_factor = (double) (1<<28) / (double) _ad9834_clock_freq;

static bool _freq_reg = 0;  // Keeps track of current freq reg, to alternate for a smooth transition

/* Sends data to the AD9834 and handles the fsync pin appropriately as per datasheet timing
*/
static inline void _transfer16(uint16_t data_to_send) {
    // gpio_put(AD9834_FSY, 0);   // FSY is treated as CS by SPI h/w
    spi_write16_blocking(spi_default, &data_to_send, 1);
    // gpio_put(AD9834_FSY, 1);
}

// Initialize the chip
void ad9834_init() {
    // Initialize Reference clock
    // 75MHz  =  150MHz / 2
    clock_gpio_init(AD9834_REF, CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLK_SYS, 2.0f);
    // 48MHz  =  48MHz / 1
    // clock_gpio_init(AD9834_REF, CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLK_ADC, 1.0f);

    // pio_init_sq(AD9834_REF_PIO, 1, AD9834_REF);
    // pio_set_sq_freq(AD9834_REF_PIO, 1, (float)_ad9834_clock_freq/1000);

    // Set pin modes & functions
    gpio_set_function(AD9834_SCK, GPIO_FUNC_SPI);
    gpio_set_function(AD9834_TXD, GPIO_FUNC_SPI);
    gpio_set_function(AD9834_FSY, GPIO_FUNC_SPI);

    gpio_set_dir(AD9834_SCK, true);
    gpio_set_dir(AD9834_TXD, true);
    gpio_set_dir(AD9834_FSY, true);

    // Init spi functionality
    spi_init(spi_default, 9600);
    spi_set_format(spi_default, 16, SPI_CPOL_1, SPI_CPHA_0, SPI_MSB_FIRST);

    // Init device
    _freq_reg = 0;  // Start with FREQ0 always for consistency
    gpio_put(AD9834_FSY, 1);    // FSYNC to 1  (initially)
    
    _transfer16(_init_code);   // Initialize the device

    _transfer16(0xC000);  // Set phase reg's to 0 offset
    _transfer16(0xE000);  // Default value... we don't care about a phase offset, yet
}

// Change the frequency as per the method in the datasheet
void ad9834_setfreq(unsigned long int freq) {

    // Calculate values for the frequecy registers
    unsigned long int freq_reg_val = (unsigned long int) (_freq_factor * freq);
    uint16_t MSW = (freq_reg_val & 0xFFFC000) >> 14;
    uint16_t LSW = (freq_reg_val & 0x3FFF);

    // Figure out address to use for frequecy register
    uint16_t freq_reg_addr = _freq_reg ? 0x8000 : 0x4000;

    _transfer16(LSW | freq_reg_addr);  // Freq reg load
    _transfer16(MSW | freq_reg_addr);

    _transfer16(_init_code | (_freq_reg * 0x800));  // Switch reg

    _freq_reg = !_freq_reg;
}

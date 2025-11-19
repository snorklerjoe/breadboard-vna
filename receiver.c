/* Module for controlling the Tayloe receiver and switching.
*/

#include "receiver.h"
#include "pio.h"

// Initializes the receiver and puts everything into a reset state
void rx_init() {
    // Set init GPIO state:
    // gpio_set_dir(LO_S0, true);
    // gpio_set_dir(LO_S1, true);
    gpio_set_dir(RX_INCT_EN, true);
    gpio_set_dir(RX_REFL_EN, true);

    gpio_put(RX_REFL_EN, true);  // Active low
    gpio_put(RX_INCT_EN, true);  // true => disable all

    // Turn on LO
    pio_init_losq(TAYLOE_PIO, 0, LO_S0, LO_S1);
    pio_set_losq_freq(TAYLOE_PIO, 0, 1000);  // 1kHz resting
}

// Sets the frequency of the LO.
// The actual LO frequency will be four times the value specified to this function,
// since the Tayloe detector requires it, but the specified frequency will be the
// offset between the RF and what is sampled by the ADC.
void rx_setfreq(unsigned long int freq) {
    pio_set_losq_freq(TAYLOE_PIO, 0, freq);
}

// Configure the receiver to receive the incident signal
void rx_set_incident() {
    gpio_put(RX_REFL_EN, true);
    gpio_put(RX_INCT_EN, false);
}

// Configure the receiver to receive the reflected signal
void rx_set_reflected() {
    gpio_put(RX_REFL_EN, false);
    gpio_put(RX_INCT_EN, true);
}

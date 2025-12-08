#ifndef PIO_SQ_H
#define PIO_SQ_H

#define PICO_CLK SYS_CLK_KHZ  // kHz

#include <pico/stdlib.h>
#include <hardware/pio.h>
#include <square.pio.h>
#include <losquare.pio.h>

// Initializes / sets freq for a single square-wave output
void pio_init_sq(PIO pio, uint sm_id, uint out_pin);
void pio_set_sq_freq(PIO pio, uint sm_id, float freq);

// Initializes / sets freq for an output of two square waves, 90deg out of phase
void pio_init_losq(PIO pio, uint sm_id, uint s0, uint s1);
float pio_set_losq_freq(PIO pio, uint sm_id, float freq);

// Resets phase of LO square wave
inline void pio_reset_losq(PIO pio, uint sm_id) {
    pio_sm_set_enabled(pio, sm_id, false);
    pio_sm_clkdiv_restart(pio, sm_id);
    pio_sm_set_enabled(pio, sm_id, true);
}


#endif
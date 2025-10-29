#ifndef PIO_SQ_H
#define PIO_SQ_H

#define PICO_CLK 150000  // kHz

#include <pico/stdlib.h>
#include <hardware/pio.h>
#include <square.pio.h>

void pio_init_sq(PIO pio, uint sm_id, uint out_pin);
void pio_set_sq_freq(PIO pio, uint sm_id, uint freq);

#endif
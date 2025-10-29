// PIO Test

#include "pio.h"

void pio_init_sq(PIO pio, uint sm_id, uint out_pin) {
    uint offset = pio_add_program(pio, &square_program);
    square_init(pio, sm_id, offset, out_pin);
}

void pio_set_sq_freq(PIO pio, uint sm_id, uint freq) {
      pio_sm_set_clkdiv(pio, sm_id, (uint) (PICO_CLK / freq) );
}

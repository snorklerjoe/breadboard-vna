// PIO Test

#include "pio.h"
// #include <pio.h>
#include <hardware/clocks.h>

void pio_init_sq(PIO pio, uint sm_id, uint out_pin) {
    uint offset = pio_add_program(pio, &square_program);
    square_init(pio, sm_id, offset, out_pin);
}

void pio_set_sq_freq(PIO pio, uint sm_id, float freq) {
      pio_sm_set_clkdiv(pio, sm_id, (uint) (PICO_CLK / freq / 2) );
}

void pio_init_losq(PIO pio, uint sm_id, uint s0, uint s1) {
    uint offset = pio_add_program(pio, &losquare_program);
    losquare_init(pio, sm_id, offset, s0);
}

float pio_set_losq_freq(PIO pio, uint sm_id, float freq) {
    float div = (((float)PICO_CLK) / freq / 4);
    uint32_t div_int;
    uint8_t div_frac8;
    pio_calculate_clkdiv8_from_float(div, &div_int, &div_frac8);
    pio_sm_set_clkdiv_int_frac(pio, sm_id, div_int, 0);

    return ((float) PICO_CLK) / (((float) div_int) + ((float) 0)/256) / 4;
}

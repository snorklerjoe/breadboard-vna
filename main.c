#include <stdio.h>
#include <stdbool.h>
#include <pico/stdlib.h>
#include <hardware/pio.h>
#include <blinky.pio.h>

#define LED_BUILTIN 25

int main() {
  stdio_init_all();
  PIO pio = pio0;
  uint state_machine_id = 0;
  uint offset = pio_add_program(pio, &blinky_program);
  printf("Initializing PIO Blinky\n\r");
  blinky_init(pio, state_machine_id, offset, LED_BUILTIN);

  pio_sm_set_clkdiv(pio, state_machine_id, 0xFFFF);
  // pio_sm_clear_fifos(pio, state_machine_id);
  // pio_sm_set_enabled(pio, state_machine_id, true);  

  while(1) {
    printf("Running PIO Blinky\n\r");
    //do nothing
  }
}
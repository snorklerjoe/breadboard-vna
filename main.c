#include <stdio.h>
#include <stdbool.h>
#include <pico/stdlib.h>
#include <hardware/pio.h>
#include <blinky.pio.h>
#include "pio.h"
#include "ad9834.h"

#define LED_BUILTIN 25

int main() {
  stdio_init_all();
  // PIO pio = pio0;
  // uint state_machine_id = 0;
  // printf("Initializing PIO Blinky\n\r");
  // pio_init_sq(pio, state_machine_id, 2);

  // uint freq = 1000;
  // while(1) {
  //   sleep_ms(1000);
  //   printf("YO: I think the freq is %i kHz\r\n", freq);
  //   freq++;
  //   pio_set_sq_freq(pio, state_machine_id, freq);
  // }

  for(int i = 7; i>0; i--) {
  sleep_ms(1000);
  printf("%i seconds left!\n\r", i);
  }

    printf("Initializing ad9834 thingy...\n\r");
  ad9834_init();
  ad9834_setfreq(0);


  while(1);

}
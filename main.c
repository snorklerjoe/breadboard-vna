#include <stdio.h>
#include <stdbool.h>
#include <pico/stdlib.h>
#include <hardware/pio.h>
#include "pio.h"
#include "ad9834.h"
#include <hardware/clocks.h>

#define LED_BUILTIN 25

static unsigned long int num_entry() {
  unsigned long int x = 0;
  char a = '\x00';
  while((a = getchar()) != '\n') {
    if(a >= '0' && a <= '9') {
      x = x * 10;
      x += a - '0';
    } else break;
  }
  return x;
}

int main() {
  stdio_init_all();
  ts_lcd_init();

  sleep_ms(1000);

  printf("Initializing AD9834...\n\r");
  ad9834_init();

  uint16_t touch_x = 100;
  uint16_t touch_y = 100;

  
  while(1) {
    unsigned long int freq = num_entry();
    printf("\n\rSetting freq to %ikHz...\n\r", freq);
    // pio_set_sq_freq(AD9834_REF_PIO, sm, (float)freq);
    ad9834_setfreq(freq*1000);
  };

}
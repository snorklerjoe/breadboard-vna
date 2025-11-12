#include <stdio.h>
#include <stdbool.h>
#include <pico/stdlib.h>
#include <hardware/pio.h>
#include <blinky.pio.h>
#include "pio.h"
#include "ad9834.h"

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

  sleep_ms(1000);

  printf("Initializing AD9834...\n\r");
  ad9834_init();

  // printf("Setting freq to 10MHz...\n\r\n\r");
  // ad9834_setfreq(10000000);

  while(1) {
    unsigned long int freq = num_entry();
    printf("\n\rSetting freq to %ikHz...\n\r", freq);
    ad9834_setfreq(freq*1000);
    //sleep_ms(1000);
  };

}
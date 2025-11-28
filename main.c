#include <stdio.h>
#include <stdbool.h>
#include <pico/stdlib.h>
#include <hardware/pio.h>
#include "pio.h"
#include "ad9834.h"
#include "receiver.h"
#include <hardware/clocks.h>

#define LED_BUILTIN 25

static unsigned long int num_entry() {
  unsigned long int x = 0;
  char a = '\x00';
  while((a = getchar()) != '\n') {
    if(a >= '0' && a <= '9') {
      putchar(a);
      x = x * 10;
      x += a - '0';
    } else break;
  }
  return x;
}

static void test_stimulus() {
  printf("Initializing AD9834...\n\r");
  ad9834_init();

  uint16_t touch_x = 100;
  uint16_t touch_y = 100;

  
  while(1) {
    unsigned long int freq = num_entry();
    printf("\n\rSetting freq to %ikHz...\n\r", freq);
    // pio_set_sq_freq(AD9834_REF_PIO, sm, (float)freq);
    ad9834_setfreq(freq*1000);
  }
}

static void test_rx() {
  printf("Initializing Receiver and AD9834...\n\r");
  ad9834_init();
  rx_init();
  // rx_set_incident();
  rx_set_reflected();


  while(1) {
    printf("SRC FREQ:  ");
    unsigned long int freq = num_entry();
    printf("\n");

    printf("LO FREQ:  ");
    unsigned long int lofreq = num_entry();
    printf("\n");

    printf("\n\rSetting AD9834 to %ikHz and LO to %ikHz...\n\r", freq, (uint16_t)rx_setfreq(lofreq));
    ad9834_setfreq(freq*1000);
  }
}

void main() {
  stdio_init_all();
  sleep_ms(1000);
  // test_stimulus();
  test_rx();
  // test_piofreq();

  // pio_init_losq(pio0, 1, 0, 1);
  // pio_set_losq_freq(pio0, 1, 1000);

  while(1);
}
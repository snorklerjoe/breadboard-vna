#include <stdio.h>
#include <stdbool.h>
#include <pico/stdlib.h>
#include <hardware/pio.h>
#include "pio.h"
#include "ad9834.h"
#include <hardware/clocks.h>
#include "TFTMaster.h"
#include "ts_lcd.h"

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

void drawCrossHair(uint16_t xcoor, uint16_t ycoor) {
    // TODO: Draw a circle at the touch point (10 pixel diameter)
    // HINT: tft_drawCircle(xcoor, ycoor, radius, color) - radius = 5 for 10px diameter
    tft_drawCircle(xcoor, ycoor, 5, ILI9340_RED);
    
    // TODO: Draw horizontal line through center (10 pixels long)
    // HINT: tft_drawLine(xcoor-5, ycoor, xcoor+5, ycoor, color)
    tft_drawLine(xcoor-5, ycoor, xcoor+5, ycoor, ILI9340_RED);
    
    // TODO: Draw vertical line through center (10 pixels long)  
    // HINT: tft_drawLine(xcoor, ycoor-5, xcoor, ycoor+5, color)
    tft_drawLine(xcoor, ycoor-5, xcoor, ycoor+5, ILI9340_RED);
    
    // TODO: Draw a center pixel for better visibility
    // HINT: tft_drawPixel(xcoor, ycoor, color)
    tft_drawPixel(xcoor, ycoor, ILI9340_WHITE);
}

int main() {
  stdio_init_all();
  ts_lcd_init();

  sleep_ms(1000);

  printf("Initializing AD9834...\n\r");
  ad9834_init();

  uint16_t touch_x = 100;
  uint16_t touch_y = 100;

  /*
  while(1) {
    unsigned long int freq = num_entry();
    printf("\n\rSetting freq to %ikHz...\n\r", freq);
    // pio_set_sq_freq(AD9834_REF_PIO, sm, (float)freq);
    ad9834_setfreq(freq*1000);
  };*/
  
  while(1) {
    drawCrossHair(touch_x,touch_y);
  }

}
#ifndef ILI9341_H
#define ILI9341_H

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <stdint.h>

// Display size
#define ILI9341_WIDTH   240
#define ILI9341_HEIGHT  320

// Commands
#define ILI9341_SWRESET 0x01
#define ILI9341_SLPOUT  0x11
#define ILI9341_DISPON  0x29
#define ILI9341_CASET   0x2A
#define ILI9341_PASET   0x2B
#define ILI9341_RAMWR   0x2C
#define ILI9341_PIXFMT  0x3A
#define ILI9341_MADCTL  0x36

// MADCTL rotation bits
#define MADCTL_MY 0x80
#define MADCTL_MX 0x40
#define MADCTL_MV 0x20
#define MADCTL_BGR 0x08

// Driver struct
typedef struct {
    spi_inst_t *spi;
    uint sck;
    uint mosi;
    uint cs;
    uint dc;
    uint rst;
} ili9341_t;

// API
void ili9341_init(ili9341_t *tft);
void ili9341_set_rotation(ili9341_t *tft, uint8_t r);
void ili9341_fill_screen(ili9341_t *tft, uint16_t color);

void ili9341_set_addr_window(ili9341_t *tft, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

void ili9341_write_pixel(ili9341_t *tft, uint16_t color);

void ili9341_box(ili9341_t *tft, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ili9341_coords(ili9341_t *tft, uint16_t *x, uint16_t *y, size_t size, uint16_t color);
void ili9341_line(ili9341_t *tft, uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t color);
void ili9341_drawOnCartGraph(ili9341_t *tft, int *xCoords, int *yCoords, size_t size, uint16_t color);
void ili9341_drawChar(ili9341_t *tft, int x, int y, char c, uint16_t fg, uint16_t bg, uint8_t scale);
void ili9341_drawString(ili9341_t *tft, int x, int y, char *c, uint16_t fg, uint16_t bg, uint8_t scale);
#endif
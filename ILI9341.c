#include <stdio.h>
#include "ILI9341.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include <math.h>

static inline void send_cmd(ili9341_t *tft, uint8_t cmd) {
    
    gpio_put(tft->dc, 0); //Next byte is command
    gpio_put(tft->cs, 0); //Active for communication
    spi_write_blocking(tft->spi, &cmd, 1);
    //pio_sm_put_blocking(tft->pio, tft->sm, cmd); //Send byte
    gpio_put(tft->cs, 1); //Prevent board from thinking next bits are part of command
}

//Same as before, but for data not commands
static inline void send_data(ili9341_t *tft, const uint8_t *data, size_t len) {
    gpio_put(tft->dc, 1);
    gpio_put(tft->cs, 0);
    //for (size_t i = 0; i < len; i++)
        //pio_sm_put_blocking(tft->pio, tft->sm, data[i]);
    spi_write_blocking(tft->spi, data, len);
    gpio_put(tft->cs, 1);
}

// Write multiple 16-bit pixels in one chip select assertion
static void ili9341_write_pixels(ili9341_t *tft, const uint16_t *colors, uint32_t count) {
    gpio_put(tft->dc, 1); //Data mode
    gpio_put(tft->cs, 0); //Only one CS assert

    for (uint32_t i = 0; i < count; i++) {
        //uint16_t color = colors[i];
        //pio_sm_put_blocking(tft->pio, tft->sm, color >> 8);// high byte
        //pio_sm_put_blocking(tft->pio, tft->sm, color & 0xFF); // low byte
        uint8_t hi = colors[i] >> 8;
        uint8_t lo = colors[i] & 0xFF;
        uint8_t buf[2] = {hi, lo};
        spi_write_blocking(tft->spi, buf, 2);
    }

    gpio_put(tft->cs, 1);// CS high 
}

//Reset board
static void hw_reset(ili9341_t *tft) {
    gpio_put(tft->rst, 0);
    sleep_ms(50);
    gpio_put(tft->rst, 1);
    sleep_ms(150);
}

void ili9341_init(ili9341_t *tft) {

    // Init pins
    gpio_init(tft->cs);  gpio_set_dir(tft->cs, GPIO_OUT); gpio_put(tft->cs, 1);
    gpio_init(tft->dc);  gpio_set_dir(tft->dc, GPIO_OUT);
    gpio_init(tft->rst); gpio_set_dir(tft->rst, GPIO_OUT);

    gpio_set_function(tft->sck,  GPIO_FUNC_SPI);
    gpio_set_function(tft->mosi, GPIO_FUNC_SPI);

    spi_init(tft->spi, 31 * 1000 * 1000); 
    hw_reset(tft);


    // Minimal init

    send_cmd(tft, ILI9341_SWRESET);
    sleep_ms(120);

    send_cmd(tft, ILI9341_SLPOUT);
    sleep_ms(120);


    uint8_t pixfmt = 0x55;  // 16-bit
    send_cmd(tft, ILI9341_PIXFMT);
    send_data(tft, &pixfmt, 1);

    send_cmd(tft, ILI9341_DISPON);
    sleep_ms(100);

}

void ili9341_set_rotation(ili9341_t *tft, uint8_t r) {
    uint8_t mad = 0;

    switch (r % 4) {
        case 0: mad = MADCTL_MX | MADCTL_BGR; break;
        case 1: mad = MADCTL_MV | MADCTL_BGR; break;
        case 2: mad = MADCTL_MY | MADCTL_BGR; break;
        case 3: mad = MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR; break;
    }

    send_cmd(tft, ILI9341_MADCTL);
    send_data(tft, &mad, 1);
}

void ili9341_set_addr_window(ili9341_t *tft, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    uint8_t caset[4] = { x >> 8, x, (x+w-1) >> 8, (x+w-1) };//column adress set
    uint8_t paset[4] = { y >> 8, y, (y+h-1) >> 8, (y+h-1) };//page adress set

    send_cmd(tft, ILI9341_CASET);
    send_data(tft, caset, 4);

    send_cmd(tft, ILI9341_PASET);
    send_data(tft, paset, 4);

    send_cmd(tft, ILI9341_RAMWR);
}

void ili9341_write_pixel(ili9341_t *tft, uint16_t color) {
    ili9341_write_pixels(tft, &color, 1);
}

void ili9341_fill_screen(ili9341_t *tft, uint16_t color) {
    ili9341_set_addr_window(tft, 0, 0, ILI9341_WIDTH, ILI9341_HEIGHT);

    // Use a small buffer for faster transfers
    #define BUF_SIZE 64
    uint16_t buffer[BUF_SIZE];
    for (int i = 0; i < BUF_SIZE; i++) buffer[i] = color;

    uint32_t pixels = ILI9341_WIDTH * ILI9341_HEIGHT;
    while (pixels > 0) {
        uint32_t n = pixels > BUF_SIZE ? BUF_SIZE : pixels;
        ili9341_write_pixels(tft, buffer, n);
        pixels -= n;
    }
}

//Advanced commands
void ili9341_box(ili9341_t *tft, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color){
    ili9341_set_addr_window(tft, x, y, w, h);
    #define BUF_SIZE 64
    uint16_t buffer[BUF_SIZE];
    for (int i = 0; i < BUF_SIZE; i++) buffer[i] = color;

    uint32_t pixels = w * h;
    while (pixels > 0) {
        uint32_t n = pixels > BUF_SIZE ? BUF_SIZE : pixels;
        ili9341_write_pixels(tft, buffer, n);
        pixels -= n;
    }
}

void ili9341_coords(ili9341_t *tft, uint16_t *x, uint16_t *y, size_t size, uint16_t color){
    for(int i = 0; i < size; i++){
        ili9341_set_addr_window(tft, *x, *y, 1, 1);

        #define BUF_SIZE 64
        uint16_t buffer[BUF_SIZE];
        for (int i = 0; i < BUF_SIZE; i++) buffer[i] = color;

        uint32_t n = 1 > BUF_SIZE ? BUF_SIZE : 1;
        ili9341_write_pixels(tft, buffer, n);
        //Buffer necessary?
        x++;
        y++;
    }
//Add bounds?
}

void ili9341_line(ili9341_t *tft, uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t color){
    int16_t xDist = xEnd - xStart;
    int16_t yDist = yEnd - yStart;
    float totalDist = sqrt(xDist*xDist + yDist*yDist);
    float xStep = xDist / totalDist;
    float yStep = yDist / totalDist;
    float xCurrent = xStart;
    float yCurrent = yStart;
    uint16_t xCoords[(int) roundf(totalDist)];
    uint16_t yCoords[(int) roundf(totalDist)];
    for(int i = 0; i < roundf(totalDist); i++){
        xCoords[i] = roundf(xCurrent);
        yCoords[i] = roundf(yCurrent);
        xCurrent += xStep;
        yCurrent += yStep;
    }
    ili9341_coords(tft, xCoords, yCoords, roundf(totalDist), color);
}

void ili9341_drawOnCartGraph(ili9341_t *tft, int *xCoords, int *yCoords, size_t size, uint16_t color){
    uint16_t xStart = 0;
    uint16_t yStart = 0;
    uint16_t xEnd = 0;
    uint16_t yEnd = 0;
    for(int i = 0; i < size - 1; i++){
        if(yCoords[i] < 240){ //Newly added for limiting
            xStart = 200 - (xCoords[i]);
            yStart = 40 + yCoords[i];
            xEnd = 200 - (xCoords[i+1]);
            yEnd = 40 + yCoords[i+1];
            if(xStart - xEnd < 170 && xStart - xEnd > -170) //Not sure about bounds on this
                ili9341_line(tft, xStart, yStart, xEnd, yEnd, color);
        }
    }
}


#include "glcdfont.c"

void ili9341_drawChar(ili9341_t *tft, int x, int y, char c, uint16_t fg, uint16_t bg, uint8_t scale){
    if (c < 32 || c > 126) return;//unsupported chars
    const uint8_t *glyph = &font[c * 5];

    for (int cx = 0; cx < 5; cx++) {
        uint8_t col = glyph[cx];

        for (int cy = 0; cy < 7; cy++) {
            uint16_t color = (col & (1 << (cy))) ? fg : bg;

            //scaled block
            ili9341_set_addr_window(tft, y + cy * scale, x + cx * scale, scale, scale);

            for(int i = 0; i < scale * scale; i++)
                ili9341_write_pixel(tft, color);
        }
    }
}

void ili9341_drawString(ili9341_t *tft, int x, int y, char *c, uint16_t fg, uint16_t bg, uint8_t scale){
    int xCurrent = x;
    while(*c != '\0'){
        ili9341_drawChar(tft, xCurrent, y, *c, fg, bg, scale);
        xCurrent = xCurrent + scale*5;
        c++;
    }
}
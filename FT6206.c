#include "FT6206.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"

void ft6206_init() {
    i2c_init(i2c0, 400000);

    gpio_set_function(8, GPIO_FUNC_I2C);  // SDA
    gpio_set_function(9, GPIO_FUNC_I2C);  // SCL

    gpio_pull_up(8);
    gpio_pull_up(9);

    uint8_t threshReg = 0x80; //Tgreshold register
    uint8_t threshVal = 0x03; //Threshold value, default is 0x12?
    i2c_write_blocking(i2c0, FT6206_ADDR, &threshReg, 1, true);
    i2c_write_blocking(i2c0, FT6206_ADDR, &threshReg, 1, false);
}

bool ft6206_touched() {
    uint8_t reg = 0x02;
    uint8_t data;

    i2c_write_blocking(i2c0, FT6206_ADDR, &reg, 1, true);
    i2c_read_blocking(i2c0, FT6206_ADDR, &data, 1, false);

    return (data & 0x0F) > 0;
}

bool ft6206_read_touch(uint16_t *x, uint16_t *y) {
    uint8_t reg = 0x02;
    uint8_t data[5];

    i2c_write_blocking(i2c0, FT6206_ADDR, &reg, 1, true);
    i2c_read_blocking(i2c0, FT6206_ADDR, data, 5, false);

    if ((data[0] & 0x0F) == 0)
        return false;

    *x = ((data[1] & 0x0F) << 8) | data[2];
    *y = ((data[3] & 0x0F) << 8) | data[4];

    return true;
}

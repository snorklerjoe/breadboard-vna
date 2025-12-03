#ifndef FT6206_H
#define FT6206_H

#include <stdint.h>
#include <stdbool.h>

#define FT6206_ADDR 0x38

void ft6206_init();
bool ft6206_touched();
bool ft6206_read_touch(uint16_t *x, uint16_t *y);

#endif

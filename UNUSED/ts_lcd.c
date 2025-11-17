// File: ts_lcd.c - Complete Implementation Guide
// Prepared by Prof. Priyank Kalgaonkar

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "stdbool.h"
#include "inttypes.h"
#include "TouchScreen.h"
#include "TFTMaster.h"
#include "ts_lcd.h"


// #define TS_MIN_Y 700      // TODO: Replace with your measured minimum X value
// #define TS_MAX_Y 3600     // TODO: Replace with your measured maximum X value  
// #define TS_MIN_X 400      // TODO: Replace with your measured minimum Y value
// #define TS_MAX_X 3600     // TODO: Replace with your measured maximum Y value

// Run the touchscreen_demo and note the min/max values when touching screen corners
// X is swapper with Y due to rotation.
// If the point is too close to the middle, decrease range, else increase.
#define TS_MIN_Y 1000
#define TS_MAX_Y 3400
#define TS_MIN_X 600
#define TS_MAX_X 4300

// Screen resolution constants (don't change these)
#define ADAFRUIT_LCD_MAX_X 239
#define ADAFRUIT_LCD_MAX_Y 319

// TODO: PRESSURE_THRESHOLD - Determine what Z value indicates a valid touch
#define PRESSURE_THRESHOLD 2000  // TODO: Adjust based on your testing

/* 
 * Linear interpolation function - COMPLETE VERSION
 * This converts raw touchscreen coordinates to LCD pixel coordinates
 * Parameters:
 *      raw : raw ADC value from touchscreen
 *      min_raw : minimum expected raw value (from calibration)
 *      max_raw : maximum expected raw value (from calibration)  
 *      min_lcd : minimum LCD coordinate (usually 0)
 *      max_lcd : maximum LCD coordinate (from screen resolution)
 */
static uint16_t linear_interpolate(uint16_t raw, uint16_t min_raw, uint16_t max_raw, 
                                   uint16_t min_lcd, uint16_t max_lcd) {
    // STEP 1: Handle edge cases where raw value is outside expected range
    // HINT: Use conditional statements to clamp the raw value within min_raw/max_raw
    
    // STEP 2: Calculate the ranges
    // HINT: You'll need to cast to int32_t to avoid overflow issues
    int32_t raw_range = (int32_t)max_raw - (int32_t)min_raw;
    int32_t lcd_range = (int32_t)max_lcd - (int32_t)min_lcd;
    
    // STEP 3: Calculate where the raw value falls within the raw range (0.0 to 1.0)
    // HINT: (raw - min_raw) gives position within raw range
    
    // STEP 4: Scale that position to the LCD range
    // HINT: Multiply the relative position by lcd_range, then divide by raw_range
    
    // STEP 5: Add the LCD minimum offset and return as uint16_t
    int32_t scaled_value = ((int32_t)raw - (int32_t)min_raw) * lcd_range / raw_range;
    return (uint16_t)(min_lcd + scaled_value);
}

bool get_ts_lcd(uint16_t *px, uint16_t *py) {
    // STEP 1: Get raw touch point data
    // HINT: Use the getPoint() function from TouchScreen.h
    // You'll need to create a TSPoint struct variable that we discussed in the lecture yesterday
    
    // STEP 2: Apply linear interpolation to convert raw X to LCD X
    // HINT: Use your linear_interpolate function with the X calibration values
    // Remember: LCD X ranges from 0 to ADAFRUIT_LCD_MAX_X
    
    // STEP 3: Apply linear interpolation to convert raw Y to LCD Y  
    // HINT: Same as step 2 but for Y coordinates
    // LCD Y ranges from 0 to ADAFRUIT_LCD_MAX_Y
    
    // STEP 4: Check if screen is actually being touched
    // HINT: Look at the Z (pressure) value - lower values mean more pressure
    // Return true if pressure is above your threshold (screen is touched)
    
    // STEP 5: Assign the interpolated values to the pointer parameters
    // HINT: Use *px = lcd_x and *py = lcd_y
    
    // STEP 6: Return appropriate boolean value based on touch detection
    
    struct TSPoint p;
    p.x = p.y = p.z = 0;
    getPoint(&p);
    //printf("RAW Touch Coords: (%d, %d)\n\r", p.x, p.y);

    
    // Apply interpolation
    uint16_t lcd_y = linear_interpolate(p.x, TS_MIN_X, TS_MAX_X, 0, ADAFRUIT_LCD_MAX_Y);
    uint16_t lcd_x = ADAFRUIT_LCD_MAX_X-linear_interpolate(p.y, TS_MIN_Y, TS_MAX_Y, 0, ADAFRUIT_LCD_MAX_X);
    
    *px = lcd_x;
    *py = lcd_y;
    
    // Check if touched (lower z value = more pressure)
    if (p.z < PRESSURE_THRESHOLD) {
        return true;
    }
    return false;
}

void ts_lcd_init() {  // Init the lcd, hardware, LCD rotation, and fill the screen with black
    adc_init();
    tft_init_hw();
    tft_begin();
    tft_setRotation(3);
    tft_fillScreen(ILI9340_BLACK);
}
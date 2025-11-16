// File: ts_lcd.h - Complete Implementation Guide
// Prepared by Prof. Priyank Kalgaonkar


#ifndef TS_LCD_H
#define TS_LCD_H

/*
 * TS_LCD.H - Touchscreen to LCD Coordinate Mapping Header File
 * 
 * This header file defines the interface for the touchscreen driver that
 * converts raw touchscreen coordinates to LCD display coordinates.
 * 
 * The driver provides calibration and linear interpolation to map the
 * analog touchscreen readings to the precise pixel positions on the LCD.
 */

#include "pico/stdlib.h"    // Standard Pico library for GPIO, timing, etc.
#include "stdbool.h"        // Boolean type support (true/false)
#include "inttypes.h"       // Integer type definitions (uint16_t, etc.)

// FUNCTION PROTOTYPES - These declare what functions are available to other files

/*
 * get_ts_lcd - Reads touchscreen and converts to LCD coordinates
 * 
 * This is the main function that applications will call to get touch input.
 * It handles reading the raw touch data, applying calibration, and converting
 * to LCD pixel coordinates.
 * 
 * Parameters:
 *   px - Pointer to store the X coordinate (0 to screen width-1)
 *   py - Pointer to store the Y coordinate (0 to screen height-1)
 * 
 * Returns:
 *   bool - true if screen is currently being touched, false otherwise
 * 
 * Usage Example:
 *   uint16_t touch_x, touch_y;
 *   if (get_ts_lcd(&touch_x, &touch_y)) {
 *       // Screen is touched - use touch_x and touch_y
 *   }
 * 
 * Implementation Notes:
 *   - The function performs linear interpolation using calibration constants
 *   - It checks touch pressure (Z value) to determine valid touches
 *   - Coordinates are clamped to valid LCD range (0-239 for X, 0-319 for Y)
 */
bool get_ts_lcd(uint16_t *px, uint16_t *py);

/*
 * ts_lcd_init - Initializes the touchscreen and LCD system
 * 
 * This function must be called once at program startup before any other
 * touchscreen functions are used. It sets up the hardware peripherals
 * and prepares the display for use.
 * 
 * Parameters: None
 * Returns: None
 * 
 * What this function typically does:
 *   - Initializes the ADC (Analog-to-Digital Converter) for touch reading
 *   - Configures GPIO pins for touchscreen control
 *   - Initializes the TFT LCD display hardware
 *   - Sets display orientation and clears the screen
 *   - Configures any required communication protocols (SPI, etc.)
 * 
 * Usage Example:
 *   int main() {
 *       ts_lcd_init();  // Must call this first!
 *       // Now you can use get_ts_lcd() safely
 *   }
 * 
 * Important: Failure to call this function first may result in hardware
 * not being properly configured and unpredictable behavior.
 */
void ts_lcd_init();

#endif // TS_LCD_H

/*
 * FILE STRUCTURE EXPLANATION:
 * 
 * #ifndef TS_LCD_H / #define TS_LCD_H / #endif
 *   - These are "include guards" that prevent the header file from being
 *     included multiple times, which would cause compilation errors.
 *   - TS_LCD_H is a unique identifier for this header file.
 * 
 * Why we use header files:
 *   - Declaration vs. Definition: Headers declare WHAT functions do (.h files),
 *     while source files define HOW they do it (.c files).
 *   - Modularity: Other files can use these functions without knowing the
 *     implementation details.
 *   - Organization: Clean separation between interface and implementation.
 * 
 * Common mistakes to avoid:
 *   - Never put function implementations (code) in header files
 *   - Only include what's necessary (don't add unused #includes)
 *   - Keep the interface minimal and well-documented
 */
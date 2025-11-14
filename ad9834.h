/* Simple library for communicating with the ad9834 board
   Loosely based on implementation from Ali Barber: https://github.com/AliBarber/AD9834/blob/master/src/AD9834.cpp
*/

#ifndef AD9834_H
#define AD9834_H

#include <hardware/pio.h>

#define AD9834_SCK 2    // Serial Clock Pin
#define AD9834_TXD 3    // Serial Data 
#define AD9834_FSY 5    // Freq sync / update strobe
#define AD9834_REF 21   // Square wave frequency reference

#define AD9834_REF_PIO pio0

// Initializes the AD9834 board and starts the reference clock.
void ad9834_init();

// Sets the frequency of the AD9834 board. Required to bring it out of RESET.
void ad9834_setfreq(unsigned long int freq);

#endif

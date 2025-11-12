/* Simple library for communicating with the ad9834 board
   Loosely based on implementation from Ali Barber: https://github.com/AliBarber/AD9834/blob/master/src/AD9834.cpp
*/

#ifndef AD9834_H
#define AD9834_H

#define AD9834_SCK 2    // Serial Clock Pin
#define AD9834_TXD 3    // Serial Data 
#define AD9834_FSY 5    // Freq sync / update strobe

void ad9834_init();

void ad9834_setfreq(unsigned long int freq);

#endif

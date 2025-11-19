/* Module for controlling the Tayloe receiver and switching.
*/

#ifndef RECEIVER_H
#define RECEIVER_H

#include <hardware/pio.h>

#define RX_INCT_EN 15  // Enable receiving the incident signal
#define RX_REFL_EN 14  // Enable receiving the reflected signal

// LO Output pins
#define LO_S0 19
#define LO_S1 20

// Internal PIO to use for the Tayloe LO
#define TAYLOE_PIO pio1

// Initializes the receiver and puts everything into a reset state
void rx_init();

// Sets the frequency of the LO.
// The actual LO frequency will be four times the value specified to this function,
// since the Tayloe detector requires it, but the specified frequency will be the
// offset between the RF and what is sampled by the ADC.
void rx_setfreq(unsigned long int freq);

// Configure the receiver to receive the incident signal
void rx_set_incident();

// Configure the receiver to receive the reflected signal
void rx_set_reflected();

#endif

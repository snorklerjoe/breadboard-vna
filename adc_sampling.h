/* Module for taking samples from the ADC, filtering a specific frequency component,
   and measuring the amplitude of said frequency component.
*/

#ifndef ADC_SAMPLING_H
#define ADC_SAMPLING_H

// Sampling and filtering parameters
#define NUM_SAMPLES 512
#define FIR_N 32
#define FIR_WIDTH 1  // kHz

// GPIO for I and Q ADC inputs
#define ADC_I 26 
#define ADC_Q 27

// Math utilities
#define MATH_PI 3.14159265
#define imin(a, b) (a < b) ? a : b
#define imax(a, b) (a > b) ? a : b

// Initialize the ADCs
void rx_adc_init();

// Gets an RMS amplitude from a pin by sampling, filtering, and calculating RMS amplitude of the filtered signal
double rx_adc_get_amplitude_blocking(int adc_pin, double freq);

// Does not apply any filtering, but takes the max - min of the signal
double rx_adc_get_pp_unfiltered_blocking(int adc_pin);

#endif

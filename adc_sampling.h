/* Module for taking samples from the ADC, filtering a specific frequency component,
   and measuring the amplitude of said frequency component.
*/

#ifndef ADC_SAMPLING_H
#define ADC_SAMPLING_H

#define NUM_SAMPLES 1024
#define FIR_N 32
#define FIR_WIDTH 1  // kHz

#define ADC_I 26 
#define ADC_Q 27

#define MATH_PI 3.14159265
#define imin(a, b) (a < b) ? a : b

void rx_adc_init();

float rx_adc_get_amplitude_blocking(int adc_pin, float freq);

#endif

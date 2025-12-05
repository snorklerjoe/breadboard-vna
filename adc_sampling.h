/* Module for taking samples from the ADC, filtering a specific frequency component,
   and measuring the amplitude of said frequency component.
*/

#ifndef ADC_SAMPLING_H
#define ADC_SAMPLING_H

#include "complex_math.h"

// Sampling and filtering parameters
#define ADC_INPUT_FREQ 50  // kHz
#define NUM_SAMPLES 250

// Hardware parameter
#define ADC_TOTAL_SAMPLE_RATE 500

// All other samples are discarded
#define NUM_SAMPLES_PROCESSED ((double) ADC_TOTAL_SAMPLE_RATE / (double) ADC_INPUT_FREQ) * 4

#define FIR_N 32
#define FIR_WIDTH 1  // kHz

// GPIO for I and Q ADC inputs
#define ADC_I 26 
#define ADC_Q 27
#define ADC_RR_MASK 0x03  // Bits 0 and 1, for 26 and 27

// Math utilities
#define MATH_PI 3.14159265359
#define imin(a, b) (a < b) ? a : b
#define imax(a, b) (a > b) ? a : b

// Initialize the ADCs
void rx_adc_init();

// Take interleaved (round-robin samples and then separate into two separate arrays) I, Q samples
// Specifically, takes NUM_SAMPLES total samples
void take_interleaved_iq_samples(double *I_samples, double *Q_samples);

// Measures a vector based on a pair of arrays of NUM_SAMPLES/2 samples of I and Q signals
double_cplx_t calc_phasor(double *I_samples, double *Q_samples);

// Gets an RMS amplitude from a pin by sampling, filtering, and calculating RMS amplitude of the filtered signal
double rx_adc_get_amplitude_blocking(int adc_pin, double freq);

// Does not apply any filtering, but takes the max - min of the signal
double rx_adc_get_pp_unfiltered_blocking(int adc_pin);

#endif

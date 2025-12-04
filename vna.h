// Meta-module to represent all VNA hardware and do calibration maths

#ifndef VNA_H
#define VNA_H

#include "receiver.h"
#include "adc_sampling.h"
#include "complex_math.h"


// Config for taking a reading
#define RDG_STEADYSTATE_DELAY_MS 2  // Number of ms to wait before assuming steady state and taking measurement
#define RDG_FREQCHANGE_DELAY_MS 10  // Number of ms to wait before assuming steady state and taking measurement
#define RDG_ADC_FREQ ADC_INPUT_FREQ  // Desired frequency to have at the ADC (kHz)

// Pin to reset the accumulator in the DDS source for phase alignment
#define SRC_RESET 16

// Actual Gamma values of cal standards
#define Gamma_Short (double_cplx_t) {-1.0, 0.0}
#define Gamma_Open (double_cplx_t) {1.0, 0.0}
#define Gamma_Load (double_cplx_t) {0.0, 0.0}  // Using an ideal 50-Ohm load
// #define Gamma_Load (double_cplx_t) {1.0/101.0, 0.0}  // Using a 51-Ohm resistor

// Error terms for one port
typedef struct {
    double_cplx_t e0;   // e00
    double_cplx_t e1;   // e11
    double_cplx_t De;   // e00*e11 - e10*e01
} error_terms_t;

// Complex number math specific to VNA measurements
#define gamma_to_s11dB(gamma) 20*log10(cplx_mag(gamma))
#define gamma_to_VSWR(gamma) ((double)cplx_mag(gamma) + 1.0)/(1.0 - (double)cplx_mag(gamma))
#define gamma_to_Z(gamma) cplx_scale(cplx_div(cplx_add(gamma, cplx_unity), cplx_sub(cplx_unity, gamma)), 50.0)


// Initializes all VNA hardware
void vna_init();

// Checks the level of the reference signal, such that 1.0 is clipping the ADC
double vna_ref_levelcheck(double freq);

// Checks the levl of the refl signal, such that 1.0 is clipping the ADC
double vna_refl_levelcheck(double freq);


/*************** SINGLE-POINT VNA MEASUREMENTS ***************/
// Sets LO as close as possible to a given frequency in kHz
// and sets the source appropriately to result in a proper ADC_FREQ.
// Returns the actual source frequency.
double vna_set_freq(uint16_t freq);

// Takes a measurement and returns the uncal'd gamma value
// Does not touch current frequency settings
double_cplx_t vna_meas_point_gamma_raw(int num_avgs);

// Returns set of error terms given measurements of short, open, load.
// These error terms are valid only at this same freq point.
error_terms_t vna_cal_point(double_cplx_t m_short, double_cplx_t m_open, double_cplx_t m_load);

// Returns calibrated Gamma from a raw Gamma and error terms
double_cplx_t vna_apply_cal_point(double_cplx_t gamma, error_terms_t err_terms);

#endif

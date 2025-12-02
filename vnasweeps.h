// Code for dealing with frequency-swept measurements,
// essentially applying the functions of vna.h to swept frequency

#ifndef VNA_SWEEPS_H
#define VNA_SWEEPS_H

#include "vna.h"
#include <stdlib.h>

// Describes the measurement setup
typedef struct {
    // Start and end frequencies in kHz
    double start_freq;
    double end_freq;
    // Number of points to store
    double num_points;
} vna_meas_setup_t;

// Stores a full VNA measurement
typedef struct {
    // Data setup
    vna_meas_setup_t *setup;         // Measurement setup used
    double *frequencies;            // Array of (actual) frequencies swept

    // Raw calibration data
    double_cplx_t *cal_short, *cal_open, *cal_load;
    // Cal data
    error_terms_t *cal;

    // Actual measurement data
    double_cplx_t *gammas_uncald;   // Array of un-calibrated Gamma values
    double_cplx_t *gammas_cald;     // Array of calibrated Gamma values
} vna_meas_t;

// Creates a new, initialized vna_meas_t instance based on a given setup
// Dynamic allocation is used, so vna_meas_deinit must follow if multiple are
// initialized in order to avoid a memory leak.
vna_meas_t vna_meas_init(vna_meas_setup_t *setup);
// Frees memory from a previously initialized instance
void vna_meas_deinit(vna_meas_t meas);

// Stores an array of frequency points and an array of uncal'd Gamma values based on a measurement setup
void vna_sweep_freq(vna_meas_t meas, double_cplx_t* gammas, uint8_t numavgs);

// Calculates error terms based on raw cal data
void vna_run_cal(vna_meas_t calmeas);

// Calculates actual Gamma values based on error terms
void vna_run_correction(vna_meas_t calmeas);

#endif

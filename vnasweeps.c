#include "vnasweeps.h"
#include <stdio.h>

// Creates a new, initialized vna_meas_t instance based on a given setup
// Dynamic allocation is used, so vna_meas_deinit must follow if multiple are
// initialized in order to avoid a memory leak.
vna_meas_t vna_meas_init(vna_meas_setup_t *setup) {
    int numpts = setup->num_points;
    vna_meas_t meas = {
        setup,  // Setup
        malloc(numpts * sizeof(double)),  // Freq points

        malloc(numpts * sizeof(double_cplx_t)),  // cal_short
        malloc(numpts * sizeof(double_cplx_t)),  // cal_open
        malloc(numpts * sizeof(double_cplx_t)),  // cal_load

        malloc(numpts * sizeof(error_terms_t)),  // cal

        malloc(numpts * sizeof(double_cplx_t)),  // gammas_uncald
        malloc(numpts * sizeof(double_cplx_t))   // gammas_cald
    };
    return meas;
}

// Frees memory from a previously initialized instance
void vna_meas_deinit(vna_meas_t meas) {
    free(meas.frequencies);
    free(meas.cal_short);
    free(meas.cal_open);
    free(meas.cal_load);
    free(meas.cal);
    free(meas.gammas_uncald);
    free(meas.gammas_cald);

    // Good practice to set things to null after freeing
    meas.setup = NULL;
    meas.frequencies = NULL;
    meas.cal_short = NULL;
    meas.cal_open = NULL;
    meas.cal_load = NULL;
    meas.cal = NULL;
    meas.gammas_uncald = NULL;
    meas.gammas_cald = NULL;
}

// Stores an array of frequency points and an array of uncal'd Gamma values based on a measurement setup
void vna_sweep_freq(vna_meas_t meas, double_cplx_t* gammas, uint8_t numavgs) {  // Assumes meas is already initialized!
    vna_meas_setup_t meas_setup = *meas.setup;
    const double approx_pts_per_decade = (double)meas_setup.num_points / log10(meas_setup.end_freq / meas_setup.start_freq);
    double log_step_size = log10(meas_setup.end_freq / meas_setup.start_freq) / (approx_pts_per_decade - 1);
    uint i = 0;
    // Store frequency and gamma for each point
    for (int i = 0; i < meas_setup.num_points; i++) {  // For each freq point
        double freq = pow(10, log10(meas_setup.start_freq) + i * log_step_size);
        printf("Freq: %f\n\r", freq);
        meas.frequencies[i] = vna_set_freq(freq);
        gammas[i] = vna_meas_point_gamma_raw(numavgs);
        i++;
    }
}

// Calculates error terms based on raw cal data
void vna_run_cal(vna_meas_t calmeas) {
    for (int i = 0; i < calmeas.setup->num_points; i++)  // Run cal function on each frequency point
        calmeas.cal[i] = vna_cal_point(calmeas.cal_short[i], calmeas.cal_open[i], calmeas.cal_load[i]);
}

// Calculates actual Gamma values based on error terms
void vna_run_correction(vna_meas_t calmeas) {
    for (int i = 0; i < calmeas.setup->num_points; i++)  // Run cal application function on each frequency point
        calmeas.gammas_cald[i] = vna_apply_cal_point(calmeas.gammas_uncald[i], calmeas.cal[i]);
}

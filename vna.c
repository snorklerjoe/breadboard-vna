// Meta-module to represent all VNA hardware and do maths

#include "vna.h"
#include "receiver.h"
#include "adc_sampling.h"
#include <pico/stdlib.h>
#include "ad9834.h"
#include <stdio.h>


// Initializes all VNA hardware
void vna_init() {
  ad9834_init();
  rx_init();
  rx_adc_init();
}

// Sets LO as close as possible to a given frequency in kHz + ADC_FREQ
// and sets the source appropriately to result in a proper ADC_FREQ.
// Returns the actual source frequency.
double vna_set_freq(uint16_t freq) {
    uint16_t lofreq_real = (uint16_t)rx_setfreq(freq + RDG_ADC_FREQ);
    uint16_t srcfreq_real = lofreq_real + RDG_ADC_FREQ;
    ad9834_setfreq(srcfreq_real*1000);
    sleep_ms(RDG_FREQCHANGE_DELAY_MS);
    return srcfreq_real;
}

// Checks the level of the reference signal, such that 1.0 is clipping the ADC
double vna_ref_levelcheck(double freq) {
    rx_set_incident();
    vna_set_freq(freq);
    sleep_ms(RDG_FREQCHANGE_DELAY_MS);
    return imax(
        rx_adc_get_pp_unfiltered_blocking(ADC_I),
        rx_adc_get_pp_unfiltered_blocking(ADC_Q)
    );
}

// Checks the levl of the refl signal, such that 1.0 is clipping the ADC
double vna_refl_levelcheck(double freq) {
    rx_set_incident();
    vna_set_freq(freq);
    sleep_ms(RDG_FREQCHANGE_DELAY_MS);
    return imax(
        rx_adc_get_pp_unfiltered_blocking(ADC_I),
        rx_adc_get_pp_unfiltered_blocking(ADC_Q)
    );
}

static double_cplx_t vna_meas_point_gamma_raw_once() {
    // Measure incident power (vector)
    rx_set_incident();
    sleep_ms(RDG_STEADYSTATE_DELAY_MS);
    double reference_I = rx_adc_get_amplitude_blocking(ADC_I, RDG_ADC_FREQ);
    double reference_Q = rx_adc_get_amplitude_blocking(ADC_Q, RDG_ADC_FREQ);
    double_cplx_t reference_rx = {reference_I, reference_Q};

    // Measure reflected power (vector)
    rx_set_reflected();
    sleep_ms(RDG_STEADYSTATE_DELAY_MS);
    double refl_I = rx_adc_get_amplitude_blocking(ADC_I, RDG_ADC_FREQ);
    double refl_Q = rx_adc_get_amplitude_blocking(ADC_Q, RDG_ADC_FREQ);
    double_cplx_t reflected_rx = {refl_I, refl_Q};

    // printf("\t|reference| = %f\t|reflected| = %f\t\n\r", cplx_mag(reference_rx), cplx_mag(reflected_rx));

    // Calculate Gamma
    double_cplx_t gamma_raw = cplx_div(
        reflected_rx,
        reference_rx
    );

    return gamma_raw;
}

double_cplx_t vna_meas_point_gamma_raw(int num_avgs) {
    // Take measurements
    double_cplx_t points[num_avgs];
    for(int i = 0; i < num_avgs; i++) {
        points[i] = vna_meas_point_gamma_raw_once();
    }

    // Find mean
    double_cplx_t sum = {0.0, 0.0};
    for(int i = 0; i < num_avgs; i++) {
        sum = cplx_add(sum, points[i]);
    }
    double_cplx_t mean = cplx_scale(sum, 1.0/num_avgs);

    if(num_avgs <= 2) return mean;

    // If enough points, drop one outlier
    int outlier_index = 0;
    double max_diff = 0;
    for(int i = 0; i < num_avgs; i++) {
        double diff = cplx_mag(cplx_sub(points[i], mean));
        if(diff > max_diff) {
            outlier_index = i;
            max_diff = diff;
        }
    }

    // Compute mean with outlier thrown out
    sum = cplx_sub(sum, points[outlier_index]);
    mean = cplx_scale(sum, 1.0/(num_avgs - 1.0));
    return mean;
}

// Returns set of error terms given measurements of short, open, load.
// These error terms are valid only at this same freq point.
// These equations find the error terms based on algebraic solutions via Cramer's rule to the
// equations given here: http://emlab.uiuc.edu/ece451/appnotes/Rytting_NAModels.pdf
// They are solved here: https://k6jca.blogspot.com/search/label/VNA%3A%2012-term%20Error%20Model
error_terms_t vna_cal_point(double_cplx_t m_short, double_cplx_t m_open, double_cplx_t m_load) {
    // Common denominator
    double_cplx_t denom = cplx_add(
        cplx_add(
            cplx_sub(
                cplx_sub(
                    cplx_mult(cplx_mult(Gamma_Short, Gamma_Open), m_short),
                    cplx_mult(cplx_mult(Gamma_Short, Gamma_Open), m_open)
                ),
                cplx_mult(cplx_mult(Gamma_Short, Gamma_Load), m_short)
            ),
            cplx_mult(cplx_mult(Gamma_Short, Gamma_Load), m_load)
        ),
        cplx_sub(
            cplx_mult(cplx_mult(Gamma_Open, Gamma_Load), m_open),
            cplx_mult(cplx_mult(Gamma_Open, Gamma_Load), m_load)
        )
    );


    // Find e00
    double_cplx_t num_e00 = cplx_add(
        cplx_add(
            cplx_sub(
                cplx_mult(cplx_mult(Gamma_Short, Gamma_Open), cplx_mult(m_short, m_load)),
                cplx_mult(cplx_mult(Gamma_Short, Gamma_Load), cplx_mult(m_short, m_open))
            ),
            cplx_sub(
                cplx_mult(cplx_mult(Gamma_Open, Gamma_Load), cplx_mult(m_short, m_open)),
                cplx_mult(cplx_mult(Gamma_Short, Gamma_Open), cplx_mult(m_open, m_load))
            )
        ),
        cplx_sub(
            cplx_mult(cplx_mult(Gamma_Short, Gamma_Load), cplx_mult(m_open, m_load)),
            cplx_mult(cplx_mult(Gamma_Open, Gamma_Load), cplx_mult(m_short, m_load))
        )
    );

    double_cplx_t e00 = cplx_div(num_e00, denom);
    
    // Find e11
    // double_cplx_t e11 = cplx_div(cplx_sub(cplx_add(cplx_scale(m_short, 2), m_open), cplx_scale(e00, 2)), m_open);
    // double_cplx_t e11 = cplx_scale(cplx_sub(m_open, m_short), 0.5);
    // double_cplx_t e11 = cplx_div(
    //     cplx_sub(cplx_scale(cplx_sub(m_short, m_load), 1),
    //     cplx_scale(cplx_sub(m_open, m_load), -1)),
    //     cplx_scale(cplx_sub(m_short, m_open), -1)
    // );

    double_cplx_t num_e11 = cplx_scale(
        cplx_add(
            cplx_add(
                cplx_sub(
                    cplx_mult(Gamma_Short, m_open),
                    cplx_mult(Gamma_Open, m_short)
                ),
                cplx_sub(
                    cplx_mult(Gamma_Load, m_short),
                    cplx_mult(Gamma_Short, m_load)
                )
            ),
            cplx_sub(
                cplx_mult(Gamma_Open, m_load),
                cplx_mult(Gamma_Load, m_open)
            )
        ),
        -1.0
    );

    double_cplx_t e11 = cplx_div(num_e11, denom);

    // Find Delta e
    // double_cplx_t De = cplx_sub(cplx_sub(e00, e11), m_short);
    // double_cplx_t De = cplx_sub(cplx_add(m_short, cplx_mult(m_short, e11)), e00);
    // double_cplx_t De = cplx_div(cplx_sub(cplx_mult(m_open, cplx_sub(m_short, m_load)), cplx_scale(cplx_mult(cplx_sub(m_open, m_load), m_short), -1)), cplx_scale(cplx_sub(m_short, m_open), -1));

    double_cplx_t num_de = cplx_scale(
        cplx_add(
            cplx_add(
                cplx_sub(
                    cplx_mult(Gamma_Short, cplx_mult(m_short, m_open)),
                    cplx_mult(Gamma_Short, cplx_mult(m_short, m_load))
                ),
                cplx_sub(
                    cplx_mult(Gamma_Open, cplx_mult(m_open, m_load)),
                    cplx_mult(Gamma_Open, cplx_mult(m_short, m_open))
                )
            ),
            cplx_sub(
                cplx_mult(Gamma_Load, cplx_mult(m_short, m_load)),
                cplx_mult(Gamma_Load, cplx_mult(m_open, m_load))
            )
        ),
        -1.0
    );

    double_cplx_t d_e = cplx_div(num_de, denom);

    // Return error vector
    return (error_terms_t){e00, e11, d_e};
}

// Returns calibrated Gamma from a raw Gamma and error terms
double_cplx_t vna_apply_cal_point(double_cplx_t gamma, error_terms_t err_terms) {
    double_cplx_t num = cplx_sub(gamma, err_terms.e0);
    double_cplx_t denom = cplx_sub(cplx_mult(gamma, err_terms.e1), err_terms.De);
    // double_cplx_t denom = cplx_sub(err_terms.e1, cplx_mult(gamma, err_terms.De));
    // double_cplx_t num = cplx_add(cplx_sub(gamma, err_terms.e0), err_terms.De);
    // double_cplx_t denom = err_terms.e1;
    return cplx_div(num, denom);
}



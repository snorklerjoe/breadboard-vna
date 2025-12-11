#include "adc_sampling.h"
#include <hardware/adc.h>
#include <hardware/dma.h>
#include "math.h"
#include <stdio.h>
#include <hardware/clocks.h>
#include <pico/stdlib.h>
#include "complex_math.h"

static int fir_n[FIR_N];
static double fir_h[FIR_N];

static uint16_t dma_buf[NUM_SAMPLES];
static double y_buf[NUM_SAMPLES + FIR_N - 1];

static inline double sinc(double x) {
    if(x == 0) return 1;
    return sin(x) / x;
}

// Puts an h[n] for a given center and width of bandpass filter into fir_h
static void gen_fir_h(double center, double width) {
    // Frequencies of step function offsets
    double f0 = center - width / 2.0;
    double f1 = center + width / 2.0;

    // Generate impulse response with which to convolve x[n]
    for(int i = 0; i < FIR_N; i++) {
        fir_n[i] = i - (FIR_N/2);
        fir_h[i] = 2.0*MATH_PI*width*sinc(2.0*MATH_PI*width*fir_n[i]) * cos(2.0*MATH_PI*center*fir_n[i]);
    }
}

// Convolves whatever is in buf with the kernel in fir_h and stores the result in y_buf
// y(n) = ∑​f(k)*h(n−k)
static void convolve() {
    // Init out buffer
    int outlen = NUM_SAMPLES + FIR_N - 1;
    for(int i = 0; i < outlen; i++) {
        y_buf[i] = 0.0;
    }

    // Generate each filtered sample
    for(int n = 0; n < outlen; n++) {
        int maxk = imin(FIR_N, n);
        for(int k = 0; k < maxk; k++) {
            y_buf[n] = y_buf[n] + ((double)dma_buf[n - k]) * fir_h[k];
        }
    }
}

void rx_adc_init() {
    adc_gpio_init(ADC_I);
    adc_gpio_init(ADC_Q);
    adc_init();
    // printf("Initialized ADC.\n\r");
}

// Take interleaved (round-robin samples and then separate into two separate arrays) I, Q samples
// Specifically, takes NUM_SAMPLES total samples
void take_interleaved_iq_samples(double *I_samples, double *Q_samples) {
    // Set up ADC for this sampling
    adc_set_round_robin(ADC_RR_MASK);
    adc_select_input(ADC_I - 26);  // Start with I signal
    adc_fifo_setup(true, true, 1, false, false);
    adc_set_clkdiv(0);  // Sample at full speed

    // Set up ADC DMA channel
    uint dma_ch = dma_claim_unused_channel(true);
    dma_channel_config cfg = dma_channel_get_default_config(dma_ch);

    // Based on https://github.com/raspberrypi/pico-examples/blob/master/adc/dma_capture/dma_capture.c
    // Reading from constant address, writing to incrementing byte addresses
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&cfg, false);
    channel_config_set_write_increment(&cfg, true);
    // Pace transfers based on availability of ADC samples
    channel_config_set_dreq(&cfg, DREQ_ADC);
    dma_channel_configure(dma_ch, &cfg,
        dma_buf,    // dst
        &adc_hw->fifo,  // src
        NUM_SAMPLES,  // transfer count
        true            // start immediately
    );

    // Start the capture
    adc_select_input(ADC_I - 26);  // Start with I signal
    adc_run(true);
    dma_channel_wait_for_finish_blocking(dma_ch);
    adc_run(false);
    adc_fifo_drain();
    dma_channel_cleanup(dma_ch);
    dma_channel_unclaim(dma_ch);

    // FOR FFT METHOD:
    // Save I and Q data, with a zero any time the other side was measuring
    // for(int i = 0; i < NUM_SAMPLES; i++) {
    //     if (i % 2 == 0)  {  // Even => I
    //         I_samples[i] = dma_buf[i];
    //         Q_samples[i] = 0.0;
    //     } else {            // Odd  => Q
    //         I_samples[i] = 0.0;
    //         Q_samples[i] = dma_buf[i];
    //     }
    // }

    // Find bias (average) of each signal to remove it
    double i_bias = 0.0;
    double q_bias = 0.0;
    int num_i_samples = 0;
    int num_q_samples = 0;
    for(int i = NUM_SAMPLES-NUM_SAMPLES_PROCESSED; i < NUM_SAMPLES; i = i + 2) {
        i_bias += dma_buf[i];
        num_i_samples++;
    }
    for(int i = NUM_SAMPLES-NUM_SAMPLES_PROCESSED + 1; i < NUM_SAMPLES; i = i + 2) {
        q_bias += dma_buf[i];
        num_q_samples++;
    }
    i_bias = (double)i_bias / num_i_samples;
    q_bias = (double)q_bias / num_q_samples;
    // i_bias = (double)(2048);   // Constant bias assumption does not work.
    // q_bias = (double)(2048);

    // Save & remove bias
    // This only operates on a region at the end of the data, to avoid
    // compensating for transients and pre-steady-state behavior
    // for(int i = 0; i < NUM_SAMPLES; i = i + 2)
        // I_samples[i/2] = ((double)dma_buf[i]) - i_bias;
    // for(int i = 1; i < NUM_SAMPLES; i = i + 2)
        // Q_samples[i/2] = ((double)dma_buf[i]) - q_bias;

    // Save I and Q data
    for(int i = 0; i < NUM_SAMPLES-1; i++) {
        if (i % 2 == 0)  {  // Even => I
            I_samples[i] = (double)dma_buf[i] - i_bias;
            I_samples[i+1] = (double)dma_buf[i] - i_bias;
            // Q_samples[i] = 0.0;
        } else {            // Odd  => Q
            // I_samples[i] = 0.0;
            Q_samples[i] = (double)dma_buf[i] - q_bias;
            Q_samples[i+1] = (double)dma_buf[i] - q_bias;
        }
    }

    // Filter I and Q data
    // gen_fir_h(((double) ADC_INPUT_FREQ) / 500, ((double) FIR_WIDTH) / 500);
    // convolve_bufs(I_samples, y_buf);
    // for(int i = 0; i < NUM_SAMPLES; i++) I_samples[i] = y_buf[i];
    // convolve_bufs(Q_samples, y_buf);
    // for(int i = 0; i < NUM_SAMPLES; i++) Q_samples[i] = y_buf[i];

}

double_cplx_t calc_phasor(double *I_samples, double *Q_samples) {
    double total_I = 0.0;
    double total_Q = 0.0;

    for(int i = NUM_SAMPLES - NUM_SAMPLES_PROCESSED; i < NUM_SAMPLES; i++) { // For each (I,Q) pair
        total_I += I_samples[i];
        total_Q += Q_samples[i];
    }

    return (double_cplx_t) {total_I / NUM_SAMPLES_PROCESSED, total_Q / NUM_SAMPLES_PROCESSED};

}

double rx_adc_get_amplitude_blocking(int adc_pin, double freq) {
    // Set up ADC FIFO
    adc_set_round_robin(0x00);
    adc_select_input(adc_pin - 26);
    adc_fifo_setup(true, true, 1, false, true);
    adc_set_clkdiv(0);

    // Set up ADC DMA channel
    uint dma_ch = dma_claim_unused_channel(true);
    dma_channel_config cfg = dma_channel_get_default_config(dma_ch);

    // Based on https://github.com/raspberrypi/pico-examples/blob/master/adc/dma_capture/dma_capture.c
    // Reading from constant address, writing to incrementing byte addresses
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&cfg, false);
    channel_config_set_write_increment(&cfg, true);
    // Pace transfers based on availability of ADC samples
    channel_config_set_dreq(&cfg, DREQ_ADC);
    dma_channel_configure(dma_ch, &cfg,
        dma_buf,    // dst
        &adc_hw->fifo,  // src
        NUM_SAMPLES,  // transfer count
        true            // start immediately
    );

    // Start the capture
    adc_run(true);
    dma_channel_wait_for_finish_blocking(dma_ch);
    adc_run(false);
    adc_fifo_drain();
    dma_channel_cleanup(dma_ch);
    dma_channel_unclaim(dma_ch);

    // Generate the FIR response with which to convolve the samples
    gen_fir_h(((double) freq) / 500, ((double) FIR_WIDTH) / 500);

    // Convolve the samples with the FIR response to get filtered data
    convolve();

    // Find scaled RMS voltage, discarding first and last FIR_N*2 many samples
    double sumsq = 0.0;
    for(int i = FIR_N; i < NUM_SAMPLES - FIR_N; i++) {
        sumsq = sumsq + y_buf[i] * y_buf[i];
    }

    double rms = sqrt(sumsq / (NUM_SAMPLES - FIR_N*2));
    return rms;  // Return rms
}


double rx_adc_get_pp_unfiltered_blocking(int adc_pin) {
    // Set up ADC FIFO
    adc_select_input(adc_pin - 26);
    adc_fifo_setup(true, true, 1, false, true);
    adc_set_clkdiv(0);

    // Set up ADC DMA channel
    uint dma_ch = dma_claim_unused_channel(true);
    dma_channel_config cfg = dma_channel_get_default_config(dma_ch);

    // Based on https://github.com/raspberrypi/pico-examples/blob/master/adc/dma_capture/dma_capture.c
    // Reading from constant address, writing to incrementing byte addresses
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&cfg, false);
    channel_config_set_write_increment(&cfg, true);
    // Pace transfers based on availability of ADC samples
    channel_config_set_dreq(&cfg, DREQ_ADC);
    dma_channel_configure(dma_ch, &cfg,
        dma_buf,            // dst
        &adc_hw->fifo,  // src
        NUM_SAMPLES,    // transfer count
        true            // start immediately
    );

    // Start the capture
    adc_run(true);
    dma_channel_wait_for_finish_blocking(dma_ch);
    adc_run(false);
    adc_fifo_drain();
    dma_channel_cleanup(dma_ch);
    dma_channel_unclaim(dma_ch);

    // Figure out max and min of signal to subtract
    uint16_t max = dma_buf[0];
    uint16_t min = dma_buf[0];

    for(int i = FIR_N; i < NUM_SAMPLES - FIR_N; i++) {
        if(dma_buf[i] > max) max = dma_buf[i];
        if(dma_buf[i] < min) min = dma_buf[i];
    }

    return (double)(max-min) / (double) (1<<12);
}

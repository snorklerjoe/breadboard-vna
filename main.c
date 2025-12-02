#include <stdio.h>
#include <stdbool.h>
#include <pico/stdlib.h>
#include <hardware/pio.h>
#include "pio.h"
#include "ad9834.h"
#include "receiver.h"
#include "adc_sampling.h"
#include <hardware/clocks.h>
#include <math.h>
#include "vna.h"

#define LED_BUILTIN 25

// Number entry from terminal
static unsigned long int num_entry() {
  unsigned long int x = 0;
  char a = '\x00';
  while((a = getchar()) != '\n') {
    if(a >= '0' && a <= '9') {
      putchar(a);
      x = x * 10;
      x += a - '0';
    } else break;
  }
  return x;
}

static void test_stimulus() {
  printf("Initializing AD9834...\n\r");
  ad9834_init();

  uint16_t touch_x = 100;
  uint16_t touch_y = 100;

  
  while(1) {
    unsigned long int freq = num_entry();
    printf("\n\rSetting freq to %ikHz...\n\r", freq);
    // pio_set_sq_freq(AD9834_REF_PIO, sm, (float)freq);
    ad9834_setfreq(freq*1000);
  }
}

static void test_rx() {
  printf("Initializing Receiver and AD9834...\n\r");
  ad9834_init();
  rx_init();
  // rx_set_incident();
  rx_set_reflected();


  while(1) {
    printf("SRC FREQ:  ");
    unsigned long int freq = num_entry();
    printf("\n");

    printf("LO FREQ:  ");
    unsigned long int lofreq = num_entry();
    printf("\n");

    printf("\n\rSetting AD9834 to %ikHz and LO to %ikHz...\n\r", freq, (uint16_t)rx_setfreq(lofreq));
    ad9834_setfreq(freq*1000);
  }
}

static void test_rx_adc() {
  const uint cal_avgs = 5;
  const uint meas_avgs = 2;

  getchar();
  getchar();
  getchar();
  getchar();
  printf("Initializing Receiver and AD9834...\n\r");
  vna_init();
  uint16_t freq = vna_set_freq(1000);
  printf("Set source frequency to %ikHz\n\r", freq);

  printf("Set short and press enter.\n\r");
  getchar();
  double_cplx_t short_meas = vna_meas_point_gamma_raw(cal_avgs);
  printf("\t%f+j%f\n\r", short_meas.a, short_meas.b);

  printf("Set open and press enter.\n\r");
  getchar();
  double_cplx_t open_meas = vna_meas_point_gamma_raw(cal_avgs);
  printf("\t%f+j%f\n\r", open_meas.a, open_meas.b);

  printf("Set load and press enter.\n\r");
  getchar();
  double_cplx_t load_meas = vna_meas_point_gamma_raw(cal_avgs);
  printf("\t%f+j%f\n\r", load_meas.a, load_meas.b);

  printf("Calibrating...\r");
  error_terms_t error_terms = vna_cal_point(short_meas, open_meas, load_meas);
  printf("Calibrated.          \n\r\n\r");


    error_terms_t err = vna_cal_point(short_meas, open_meas, load_meas);

  double_cplx_t cal_short = vna_apply_cal_point(short_meas, err);
  double_cplx_t cal_open  = vna_apply_cal_point(open_meas,  err);
  double_cplx_t cal_load  = vna_apply_cal_point(load_meas,  err);

  printf("cal short: |Γ|=%f ∠%f°\n\r", cplx_mag(cal_short), cplx_ang_deg(cal_short));
  printf("cal open : |Γ|=%f ∠%f°\n\r", cplx_mag(cal_open),  cplx_ang_deg(cal_open));
  printf("cal load : |Γ|=%f ∠%f° (expected %f ∠0°)\n\r", cplx_mag(cal_load), cplx_ang_deg(cal_load), cplx_mag(Gamma_Load));
  printf("Calibration Error Terms:  (e00=%f, e11=%f, Delta e=%f)\n\r\n\r", error_terms.e0, error_terms.e1, error_terms.De);

  while(1) {
    double_cplx_t gamma_raw = vna_meas_point_gamma_raw(meas_avgs);
    double_cplx_t gamma_cald = vna_apply_cal_point(gamma_raw, error_terms);
    // printf("Raw Γ = %f + j%f\t = %f ∠ %f\tCal'd: Γ ~= %f ∠ %fdeg\t\r", gamma_raw.a, gamma_raw.b, cplx_mag(gamma_raw), cplx_ang(gamma_raw),  cplx_mag(gamma_cald), 180*cplx_ang(gamma_cald)/MATH_PI);
    printf("Cal'd: Γ = %f ∠ %fdeg            \n\r", cplx_mag(gamma_cald), 180*cplx_ang(gamma_cald)/MATH_PI);
    printf("|S11|dB = %f,  \tVSWR = %f,  \tZ = %f+j%f       ", gamma_to_s11dB(gamma_cald), gamma_to_VSWR(gamma_cald), gamma_to_Z(gamma_cald));
    printf("\033[1A\r\r");
  }
}

void main() {
  stdio_init_all();
  sleep_ms(1000);
  // test_stimulus();
  // test_rx();
  // test_piofreq();
  test_rx_adc();

  // pio_init_losq(pio0, 1, 0, 1);
  // pio_set_losq_freq(pio0, 1, 1000);

  while(1);
}
#include <stdio.h>
#include "pico/stdlib.h"
#include "ILI9341.h"
#include "FT6206.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include <math.h>

#include <pico/mutex.h>
#include <pico/multicore.h>
#include "vna.h"
#include "vnasweeps.h"
#include "complex_math.h"


// #define TEST_MODE   // When defined, continuously measures at 1MHz without ui


// Number of measurements to average together, discarding one outlier
const uint meas_avgs = 1;  // For normal measurements
const uint cal_avgs = 3;   // For initial calibration

// Number of points in a measurement
#define num_points 20
// Colors for the graph traces
#define phaseColor 0x00FF
#define lossColor 0xFF00

// Stores the setup of the measurement
vna_meas_setup_t measurement_setup;

// Stores the data from calibration and measurement
vna_meas_t measurement_data;

// Stores the data that actually get graphed
double *graph_frequencies;     // An array (size `num_points`)
double graph_return_loss_dB[num_points];
double graph_phase_deg[num_points];

// Global coordinate buffers
uint16_t a, b;

// TFT Display setup
ili9341_t tft = {
    .spi = spi1,
    .cs  = 13,
    .dc  = 12,
    .rst = 7,
    .mosi = 11,
    .sck = 10
};

// Mutex for data arrays that get graphed
mutex_t data_mutex;
// Tells when new data is available
bool change = false;

// Test function for now...
void test() {
    vna_set_freq(1000);  // 1MHz
    while(1) {
        printf("\n\r");
        vna_meas_point_gamma_raw(1);
    }
}

//For formatting loss coordinate values to our graph
void lossConversion(int *coords, size_t length){
    for(int i = 0; i < length; i++){
        *coords = 4*(*coords + 40);
        coords++;
    }
}

//For formatting phase coordinate values to our graph
void phaseConversion(int *coords, size_t length){
    for(int i = 0; i < length; i++){
        *coords = 0.5*(*coords + 180);
        coords++;
    }
}

// Initialize VNA hardware
void init_vna() {
    // Initialize hardware
    vna_init();

    // Define masurement setup
    measurement_setup = (vna_meas_setup_t){
        // Start (kHz)
        (double) 250,
        // End (kHz)
        (double) 12500,
        // Num Points
        (uint) num_points
    };

    // Initialize measurement data arrays
    measurement_data = vna_meas_init(&measurement_setup);
    
    // Copy pointer to frequencies array
    graph_frequencies = measurement_data.frequencies;
}

// Calibrate
void calibration_routine() {
    // UI: Ask the user to connect a SHORT
    // Wait for them to press a button or press the screen or something
    ili9341_fill_screen(&tft, 0x0000);
    ili9341_box(&tft, 100, 100, 20, 80, 0x0000);
    ili9341_drawString(&tft, 100, 100, "Connect Short  ", 0xFFFF, 0x0000, 2);
    while (1){
        if (ft6206_read_touch(&a, &b)){
            break;
        }
    }

    sleep_ms(100);
    ili9341_drawString(&tft, 100, 150, "Loading...", 0xFFFF, 0x0000, 1);
    vna_sweep_freq(measurement_data, measurement_data.cal_short, cal_avgs);
    ili9341_box(&tft, 150, 100, 100, 100, 0x0000);

    // UI: Ask the user to connect a OPEN
    // Wait for them to press a button or press the screen or something
    ili9341_drawString(&tft, 100, 100, "Connect Open   ", 0xFFFF, 0x0000, 2);
    while (1){
        if (ft6206_read_touch(&a, &b)){
            break;
        }
    }

    sleep_ms(100);
    ili9341_drawString(&tft, 100, 150, "Loading...", 0xFFFF, 0x0000, 1);
    vna_sweep_freq(measurement_data, measurement_data.cal_open, cal_avgs);
    ili9341_box(&tft, 150, 100, 100, 100, 0x0000);


    // UI: Ask the user to connect a LOAD
    // Wait for them to press a button or press the screen or something
    ili9341_drawString(&tft, 100, 100, "Connect Load  ", 0xFFFF, 0x0000, 2);
    while (1){
        if (ft6206_read_touch(&a, &b)){
            break;
        }
    }
    ili9341_drawString(&tft, 100, 150, "Loading...", 0xFFFF, 0x0000, 1);
    vna_sweep_freq(measurement_data, measurement_data.cal_load, cal_avgs);

    // Do calibration 3-term error model maths
    vna_run_cal(measurement_data);
}

// Takes a measurement
void take_measurement() {
    // Take measurement and put it in the measurement_data arrays
    vna_sweep_freq(measurement_data, measurement_data.gammas_uncald, meas_avgs);

    // Apply calibration
    vna_run_correction(measurement_data);

    // Prep data for graphing
    mutex_enter_blocking(&data_mutex);
    for(uint i = 0; i < num_points; i++) {
        // Save the phase angle of the reflection coefficient
        graph_phase_deg[i] = cplx_ang_deg(
            measurement_data.gammas_cald[i]
        );

        // Save the return loss
        graph_return_loss_dB[i] = gamma_to_s11dB(
            measurement_data.gammas_cald[i]
        );
    }
    mutex_exit(&data_mutex);

    // Print out Impedance vs. Freq
    double prevfreq = 0.0;
    for(uint i = 0; i < num_points; i++) {
        double freq = measurement_data.frequencies[i]/1000;
        if (freq == prevfreq) continue;

        double_cplx_t gamma = cplx_scale(measurement_data.gammas_cald[i], -1.0);
        double_cplx_t impedance = cplx_scale(cplx_div(
            cplx_add(cplx_unity, gamma),
            cplx_sub(cplx_unity, gamma)
        ), 50.0);
        printf("Freq: %fMHz  \tZ=(%f + j%f)    \n\r", freq, impedance.a, impedance.b);
        printf("             \tGamma = %f \t< %f    \n\r", cplx_mag(gamma), cplx_ang_deg(gamma));
        prevfreq = freq;
    }
    printf("\n\r");
}

// Measurement task to run on core 2
void meas_core_task() {
    while(1) {
        take_measurement();
        change = true;
    }
}

int main() {
    stdio_init_all();
    init_vna();

    // Run test mode if compiled with set
    #ifdef TEST_MODE
    test();
    #endif

    ili9341_init(&tft);
    ft6206_init();

    mutex_init(&data_mutex);  // Initialize mutex for multicore
    
    int yLossCoords[num_points];
    int xCoords[num_points];
    int yPhaseCoords[num_points];
    
    int PPD = 70; //Pixels per decade
    bool LOSS = true; //Display loss
    bool PHASE = true; //Display phase

    bool MENU = true;
    ili9341_fill_screen(&tft, 0x0000);

    // RUN CALIBRATION:
    calibration_routine();

    // Start measurement loop in the background:
    multicore_launch_core1(meas_core_task);

    ili9341_box(&tft, 0, 300, 20, 20, 0x0000);
    while (1){
        if (ft6206_read_touch(&a, &b)){ //Checking if button that switches between Menu and Graph is pushed
            //Not accurate ranges for the box, but makes things easier
            if(a <= 40 && b <= 40){
                MENU = !MENU;
                change = true;
                sleep_ms(100);
            }
        }

        
        if(MENU){ //In Menu screen

            if(change){
                ili9341_fill_screen(&tft, 0x0000);
                ili9341_box(&tft, 0, 300, 20, 20, 0xFFFF);
                ili9341_drawString(&tft, 140, 0, "MENU", 0xFFFF, 0x0000, 2);
                
                ili9341_drawString(&tft, 50, 50, "PPD", 0xFFFF, 0x0000, 2);
                ili9341_drawString(&tft, 150, 50, "TOG", 0xFFFF, 0x0000, 2);

                ili9341_box(&tft, 80, 50, 20, 30, 0x0000);
                ili9341_drawString(&tft, 50, 80, "50", 0xFFFF, 0x0000, 2);
                ili9341_box(&tft, 110, 50, 20, 30, 0x0000);
                ili9341_drawString(&tft, 50, 110, "60", 0xFFFF, 0x0000, 2);
                ili9341_box(&tft, 140, 50, 20, 30, 0x0000);
                ili9341_drawString(&tft, 50, 140, "70", 0xFFFF, 0x0000, 2);
                ili9341_box(&tft, 170, 50, 20, 30, 0x0000);
                ili9341_drawString(&tft, 50, 170, "80", 0xFFFF, 0x0000, 2);
                ili9341_box(&tft, 200, 50, 20, 30, 0x0000);
                ili9341_drawString(&tft, 50, 200, "90", 0xFFFF, 0x0000, 2);

                ili9341_box(&tft, 80, 150, 20, 50, 0x0000);
                ili9341_drawString(&tft, 150, 80, "LOSS", 0xFFFF, 0x0000, 2);
                ili9341_box(&tft, 110, 150, 20, 50, 0x0000);
                ili9341_drawString(&tft, 150, 110, "PHAS", 0xFFFF, 0x0000, 2);
                ili9341_box(&tft, 140, 150, 20, 50, 0x0000);
                ili9341_drawString(&tft, 150, 140, "BOTH", 0xFFFF, 0x0000, 2);
                
                change = false;
            }

            if (ft6206_read_touch(&a, &b)){
                if(b <= 260 && b >= 230){ //Freq buttons
                    if(a >= 80 && a <= 100){
                        for(int i = 0; i < num_points; i++){
                            xCoords[i] = 50*xCoords[i]/PPD;
                        }
                        PPD = 50;
                        ili9341_box(&tft, 80, 50, 20, 30, 0x001F);
                        ili9341_drawString(&tft, 50, 80, "50", 0xFFFF, 0x0000, 2);
                        sleep_ms(100);
                    }
                    else if(a >= 110 && a <= 130){
                        for(int i = 0; i < num_points; i++){
                            xCoords[i] = 60*xCoords[i]/PPD;
                        }
                        PPD = 60;
                        ili9341_box(&tft, 110, 50, 20, 30, 0x001F);
                        ili9341_drawString(&tft, 50, 110, "60", 0xFFFF, 0x0000, 2);
                        sleep_ms(100);
                    }
                    else if(a >= 140 && a <= 160){
                        for(int i = 0; i < num_points; i++){
                            xCoords[i] = 70*xCoords[i]/PPD;
                        }
                        PPD = 70;
                        ili9341_box(&tft, 140, 50, 20, 30, 0x001F);
                        ili9341_drawString(&tft, 50, 140, "70", 0xFFFF, 0x0000, 2);
                        sleep_ms(100);
                    }
                    else if(a >= 170 && a <= 190){
                        for(int i = 0; i < num_points; i++){
                            xCoords[i] = 80*xCoords[i]/PPD;
                        }
                        PPD = 80;
                        ili9341_box(&tft, 170, 50, 20, 30, 0x001F);
                        ili9341_drawString(&tft, 50, 170, "80", 0xFFFF, 0x0000, 2);
                        sleep_ms(100);
                    }
                    else if(a >= 200 && a <= 220){
                        for(int i = 0; i < num_points; i++){
                            xCoords[i] = 90*xCoords[i]/PPD;
                        }
                        PPD = 90;
                        ili9341_box(&tft, 200, 50, 20, 30, 0x001F);
                        ili9341_drawString(&tft, 50, 200, "90", 0xFFFF, 0x0000, 2);
                        sleep_ms(100);
                    }
                }
                else if(b <= 160 && b >= 130){ //Toggle buttons
                    if(a >= 80 && a <= 100){
                        ili9341_box(&tft, 80, 150, 20, 50, 0x001F);
                        ili9341_drawString(&tft, 150, 80, "LOSS", 0xFFFF, 0x0000, 2);
                        LOSS = true;
                        PHASE = false;
                        sleep_ms(100);
                    }
                    else if(a >= 110 && a <= 130){
                        ili9341_box(&tft, 110, 150, 20, 50, 0x001F);
                        ili9341_drawString(&tft, 150, 110, "PHAS", 0xFFFF, 0x0000, 2);
                        LOSS = false;
                        PHASE = true;
                        sleep_ms(100);
                    }
                    else if(a >= 140 && a <= 160){
                        ili9341_box(&tft, 140, 150, 20, 50, 0x001F);
                        ili9341_drawString(&tft, 150, 140, "BOTH", 0xFFFF, 0x0000, 2);
                        LOSS = true;
                        PHASE = true;
                        sleep_ms(100);
                    }
                }
            }
        }

        else{ //In Graph screen
            if(change){
                // Acknowledge change
                change = false;

                // Acquire lock
                mutex_enter_blocking(&data_mutex);

                // Copy data
                for(int i = 0; i < num_points; i++){
                    //x is y
                    yLossCoords[i] = graph_return_loss_dB[i];
                    yPhaseCoords[i] = graph_phase_deg[i];
                    xCoords[i] = PPD*log10(graph_frequencies[i]*1000)-5*PPD;  // graph_frequencies is in kHz, want Hz
                }

                // Release lock
                mutex_exit(&data_mutex);

                lossConversion(yLossCoords, num_points);
                phaseConversion(yPhaseCoords, num_points);

                ili9341_fill_screen(&tft, 0x0000);
                ili9341_line(&tft, 200, 40, 200, 280, 0x07E0);
                ili9341_line(&tft, 200, 40, 0, 40, 0x07E0);
                ili9341_line(&tft, 200, 280, 0, 280, 0x07E0); //Make sure this works
                char str[5];
                int j = 40;
                for(int i = 5; j < 280; i++){//changed from 320
                    int temp = pow(10,i)/1000;
                    sprintf(str, "%d", temp);
                    ili9341_drawString(&tft, j, 201, str, 0xFFFF, 0x0000, 1);
                    ili9341_line(&tft, 200, j, 0, j, 0x07E0);
                    j = j + PPD;
                }

                for(int i = -40; i < 6; i = i + 5){
                    sprintf(str, "%d", i);
                    ili9341_drawString(&tft, 25, 193 - 4*(40+i), str, lossColor, 0x0000, 1);
                    ili9341_line(&tft, 200 - 4*(40+i), 40, 200 - 4*(40+i), 280, 0x07E0);
                }

                for(int i = -180; i < 200; i = i + 40){
                    sprintf(str, "%d", i);
                    ili9341_drawString(&tft, 280, 193 - 0.5*(180+i), str, phaseColor, 0x0000, 1);
                }
                if(LOSS) ili9341_drawOnCartGraph(&tft, yLossCoords, xCoords, sizeof(xCoords)/sizeof(xCoords[0]), lossColor);
                if(PHASE) ili9341_drawOnCartGraph(&tft, yPhaseCoords, xCoords, sizeof(xCoords)/sizeof(xCoords[0]), phaseColor);

                

                ili9341_drawString(&tft, 140, 220, "Frequency(kHz)", 0xFFFF, 0x0000, 1);
                ili9341_drawString(&tft, 0, 220, "Loss(dB)", 0xFFFF, 0x0000, 1);
                ili9341_drawString(&tft, 280, 220, "Phase", 0xFFFF, 0x0000, 1);

                ili9341_box(&tft, 0, 300, 20, 20, 0xFFFF);
                change = false;
            }
        }
    }
}
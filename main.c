#include <stdio.h>
#include "pico/stdlib.h"
#include "ili9341.h"
#include "ft6206.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include <math.h>

int main() {
    stdio_init_all();


    ili9341_t tft = {
        .spi = spi0,
        .cs  = 17,
        .dc  = 16,
        .rst = 20,
        .mosi = 19,
        .sck = 18
    };


    ili9341_init(&tft);
    ft6206_init();
    int yLossCoords[100];
    int xCoords[100];
    int yPhaseCoords[100];
    for(int i = 0; i < 100; i++){
        //x is y
        yLossCoords[i] = i - 40;
        yPhaseCoords[i] = -1*i;
        xCoords[i] = 10*log10(i);
    }

    int PPD = 10; //Pixels per decade

    uint16_t a, b;
    bool MENU = true;
    bool change = false;
    ili9341_fill_screen(&tft, 0xF800);
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
                ili9341_fill_screen(&tft, 0xF800);
                ili9341_box(&tft, 0, 300, 20, 20, 0x0000);
                ili9341_drawString(&tft, 140, 0, "MENU", 0x0000, 0xFFFF, 2);
                
                ili9341_drawString(&tft, 50, 50, "PPD", 0x0000, 0xFFFF, 2);
                ili9341_drawString(&tft, 150, 50, "TOG", 0x0000, 0xFFFF, 2);

                ili9341_box(&tft, 80, 50, 20, 30, 0x0000);
                ili9341_drawString(&tft, 50, 80, "10", 0x0000, 0xFFFF, 2);
                ili9341_box(&tft, 110, 50, 20, 30, 0x0000);
                ili9341_drawString(&tft, 50, 110, "20", 0x0000, 0xFFFF, 2);
                ili9341_box(&tft, 140, 50, 20, 30, 0x0000);
                ili9341_drawString(&tft, 50, 140, "30", 0x0000, 0xFFFF, 2);
                ili9341_box(&tft, 170, 50, 20, 30, 0x0000);
                ili9341_drawString(&tft, 50, 170, "40", 0x0000, 0xFFFF, 2);
                ili9341_box(&tft, 200, 50, 20, 30, 0x0000);
                ili9341_drawString(&tft, 50, 200, "50", 0x0000, 0xFFFF, 2);

                ili9341_box(&tft, 80, 150, 20, 50, 0x0000);
                ili9341_drawString(&tft, 150, 80, "GAIN", 0x0000, 0xFFFF, 2);
                ili9341_box(&tft, 110, 150, 20, 50, 0x0000);
                ili9341_drawString(&tft, 150, 110, "PHAS", 0x0000, 0xFFFF, 2);
                ili9341_box(&tft, 140, 150, 20, 50, 0x0000);
                ili9341_drawString(&tft, 150, 140, "BOTH", 0x0000, 0xFFFF, 2);
                
                change = false;
            }

            if (ft6206_read_touch(&a, &b)){
                if(b <= 260 && b >= 230){
                    if(a >= 80 && a <= 100){
                        for(int i = 0; i < sizeof(xCoords)/sizeof(xCoords[0]); i++){
                            xCoords[i] = 10*xCoords[i]/PPD;
                        }
                        PPD = 10;
                        ili9341_box(&tft, 80, 50, 20, 30, 0x001F);
                        ili9341_drawString(&tft, 50, 80, "10", 0x0000, 0xFFFF, 2);
                        sleep_ms(100);
                    }
                    else if(a >= 110 && a <= 130){
                        for(int i = 0; i < 100; i++){
                            xCoords[i] = 20*xCoords[i]/PPD;
                        }
                        PPD = 20;
                        ili9341_box(&tft, 110, 50, 20, 30, 0x001F);
                        ili9341_drawString(&tft, 50, 110, "20", 0x0000, 0xFFFF, 2);
                        sleep_ms(100);
                    }
                    else if(a >= 140 && a <= 160){
                        for(int i = 0; i < 100; i++){
                            xCoords[i] = 30*xCoords[i]/PPD;
                        }
                        PPD = 30;
                        ili9341_box(&tft, 140, 50, 20, 30, 0x001F);
                        ili9341_drawString(&tft, 50, 140, "30", 0x0000, 0xFFFF, 2);
                        sleep_ms(100);
                    }
                    else if(a >= 170 && a <= 190){
                        for(int i = 0; i < 100; i++){
                            xCoords[i] = 40*xCoords[i]/PPD;
                        }
                        PPD = 40;
                        ili9341_box(&tft, 170, 50, 20, 30, 0x001F);
                        ili9341_drawString(&tft, 50, 170, "40", 0x0000, 0xFFFF, 2);
                        sleep_ms(100);
                    }
                    else if(a >= 200 && a <= 220){
                        for(int i = 0; i < 100; i++){
                            xCoords[i] = 50*xCoords[i]/PPD;
                        }
                        PPD = 50;
                        ili9341_box(&tft, 200, 50, 20, 30, 0x001F);
                        ili9341_drawString(&tft, 50, 200, "50", 0x0000, 0xFFFF, 2);
                        sleep_ms(100);
                    }
                }
            }
        }

        else{ //In Graph screen
            if(change){
                ili9341_fill_screen(&tft, 0xF800);
                ili9341_line(&tft, 200, 40, 200, 280, 0x07E0);
                ili9341_line(&tft, 200, 40, 0, 40, 0x07E0);
                ili9341_line(&tft, 200, 280, 0, 280, 0x07E0); //Make sure this works
                char str[5];
                int j = 40;
                for(int i = 0; j < 280; i++){//changed from 320
                    sprintf(str, "%d", i);
                    ili9341_drawString(&tft, j, 200, str, 0x0000, 0xFFFF, 1);
                    ili9341_line(&tft, 200, j, 0, j, 0x07E0);
                    j = j + PPD;
                }

                for(int i = -40; i < 6; i = i + 5){
                    sprintf(str, "%d", i);
                    ili9341_drawString(&tft, 25, 193 - 4*(40+i), str, 0x0000, 0xFFFF, 1);
                    ili9341_line(&tft, 200 - 4*(40+i), 40, 200 - 4*(40+i), 280, 0x07E0);
                }

                for(int i = -180; i < 200; i = i + 40){
                    sprintf(str, "%d", i);
                    ili9341_drawString(&tft, 280, 193 - 0.5*(180+i), str, 0x0000, 0xFFFF, 1);
                }
                
                ili9341_drawOnCartGraph(&tft, yLossCoords, xCoords, 100, 0x001F);

                

                ili9341_drawString(&tft, 140, 220, "Frequency(Hz)", 0x0000, 0xFFFF, 1);
                ili9341_drawString(&tft, 0, 220, "Gain(dB)", 0x0000, 0xFFFF, 1);

                ili9341_box(&tft, 0, 300, 20, 20, 0x0000);
                change = false;
            }
        }
    }
}
/*
//For formatting loss coordinate values to our graph
void lossConversion(int *coords, size_t length){
    for(int i = 0; i < length; i++){

    }
}

*/
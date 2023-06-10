#include <stdio.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/cyw43_arch.h"
#include "hardware/spi.h"
#include "include/generalOps.h"
#include "include/LCDops.h"

#define SPI_PORT spi0
#define MISO_PIN 16
#define CS_PIN 17
#define SCLK_PIN 18
#define MOSI_PIN 19
#define GREEN_LED_PIN 26
#define RED_LED_PIN 27

int LCDpins[14] = {7, 6, 5, 4, 3, 2, 1, 0, 8, 10, 9, 16, 2};
int32_t t_fine;
uint16_t dig_T1;
int16_t dig_T2, dig_T3;

int32_t compTemp(int32_t adc_T) {
    int32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((int32_t) dig_T1 << 1))) * ((int32_t) dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t) dig_T1)) * ((adc_T >> 4) - ((int32_t) dig_T1))) >> 12) * ((int32_t) dig_T3)) >> 14;

    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
   
    return T;
}

void read_temp_comp() {
    uint8_t buffer[6], reg;

    reg = 0x88 | 0x80;
    gpio_put(CS_PIN, 0);
    spi_write_blocking(SPI_PORT, &reg, 1);
    spi_read_blocking(SPI_PORT, 0, buffer, 6);
    gpio_put(CS_PIN, 1);

    dig_T1 = buffer[0] | (buffer[1] << 8);
    dig_T2 = buffer[2] | (buffer[3] << 8);
    dig_T3 = buffer[4] | (buffer[5] << 8);
}

void bmp_280_reading(float *temp_c) {
    spi_init(SPI_PORT, 500000); // Initialise spi0 at 500kHz
    
    // init gpio pin for spi
    gpio_set_function(MISO_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SCLK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(MOSI_PIN, GPIO_FUNC_SPI);

    // configure cs pin
    gpio_init(CS_PIN); 
    gpio_set_dir(CS_PIN, GPIO_OUT);
    gpio_put(CS_PIN, 1); 

    read_temp_comp(); // Read factory calibration/compensation values

    uint8_t data[2]; 
    data[0] = 0xF4 & 0x7F; 
    data[1] = 0x27; 
    gpio_put(CS_PIN, 0); 
    spi_write_blocking(SPI_PORT, data, 2); 
    gpio_put(CS_PIN, 1); 

    int32_t temperature, rawtemp;
    uint8_t reg, buffer[3];

    reg = 0xFA | 0X80;
    gpio_put(CS_PIN, 0);
    spi_write_blocking(SPI_PORT, &reg, 1);
    spi_read_blocking(SPI_PORT, 0, buffer, 3);
    gpio_put(CS_PIN, 1);

    rawtemp = ((uint32_t) buffer[0] << 12) | ((uint32_t) buffer[1] << 4) | ((uint32_t) buffer[2] >> 4);
    temperature = compTemp(rawtemp);
    *temp_c = temperature / 100.00; 
}

void light_led(float temp) {
    gpio_init(RED_LED_PIN);
    gpio_set_dir(RED_LED_PIN, GPIO_OUT);
    gpio_init(GREEN_LED_PIN);
    gpio_set_dir(GREEN_LED_PIN, GPIO_OUT);

    if (temp <= 30) {
        gpio_put(RED_LED_PIN, 0);
        gpio_put(GREEN_LED_PIN, 1);
    }
    else {
        gpio_put(GREEN_LED_PIN, 0);
        gpio_put(RED_LED_PIN, 1);
    }
}

int main(void) {
    bi_decl(bi_program_description("Weather Station using BMP280 sensor and HD44780 LCD"));
    stdio_init_all();

    for(int pin = 0; pin < 11; pin++){
        gpio_init(LCDpins[pin]);
        gpio_set_dir(LCDpins[pin], 1);
        gpio_put(LCDpins[pin], 0);
    }

    float temp;

    LCDinit();
    LCDclear();
    LCDgoto("00");
    LCDsendRawInstruction(0,0,"00001100");
    
    while (1) {
        char temp_val[7];
        char final_string[13] = "Temp: ";

        bmp_280_reading(&temp);    
        sprintf(temp_val, "%.2f", temp);
        strcat(final_string, temp_val);

        LCDclear();
        printf("%s\n", final_string);
        light_led(temp);
        LCDwriteMessage(final_string);
		LCDwriteMessage(" C");
        printf("\n");
        sleep_ms(60000);
    }

    return 0;
}

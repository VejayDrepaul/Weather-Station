#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/cyw43_arch.h"
#include "hardware/spi.h"
#include "generalOps.h"
#include "LCDops.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

int LCDpins[14] = {7, 6, 5, 4, 3, 2, 1, 0, 8, 10, 9, 16, 2};

int main(void) {
    bi_decl(bi_program_description("This is a work-in-progress example of interfacing with LCD Displays using HD44780 chips on the Raspberry Pi Pico!"));

    stdio_init_all();

    //Initialize all needed pins as defined in LCDpins, set them as
    // outputs and then pull them low
    for(int gpio = 0; gpio < 11; gpio++){
        gpio_init(LCDpins[gpio]);
        gpio_set_dir(LCDpins[gpio], true);
        gpio_put(LCDpins[gpio], false);
    }

    //Initialize and clear the LCD, prepping it for characters / instructions
    LCDinit();
    LCDclear();
    LCDgoto("02");
    LCDsendRawInstruction(0,0,"00001100");
    LCDwriteMessage("Hello World!");

    return 0;
}

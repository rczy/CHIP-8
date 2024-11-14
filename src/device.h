#ifndef DEVICE_H
#define DEVICE_H

#include "chip8.h"
#include "display.h"

#define DEFAULT_IPF 9

typedef struct device_t {
    // CHIP-8 interpreter
    chip8_t *chip_8;
    // display
    display_t *display;
    // ROM file path
    char *rom_path;
    // instructions per frame
    uint16_t ipf;
    // ticks for 1 Hz timer
    uint32_t t1;
    // ticks for 60 Hz timer
    uint32_t t60;
    // framecount of last cycle
    uint16_t frames;
    // is running?
    uint8_t running;
} device_t;

device_t *device_init(char *rom_path, uint16_t ipf);
rom_ld_t device_start(device_t *device);
void device_iterate(device_t *device);
void device_destroy(device_t **device);

#endif
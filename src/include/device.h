#ifndef DEVICE_H
#define DEVICE_H

#include "args.h"
#include "chip8.h"
#include "display.h"
#include "beeper.h"

typedef struct device_t {
    // CHIP-8 interpreter
    chip8_t *chip_8;
    // display
    display_t *display;
    // beeper
    beeper_t *beeper;
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

device_t *device_init(args_t *args);
rom_ld_t device_start(device_t *device);
void device_iterate(device_t *device);
void device_destroy(device_t **device);

#endif

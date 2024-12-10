#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <SDL2/SDL.h>
#include "include/device.h"
#include "include/chip8.h"
#include "include/input.h"
#include "include/display.h"
#include "include/beeper.h"

device_t *device_init(args_t *args)
{
    srand(time(NULL));
    device_t *device = malloc(sizeof(device_t));
    device->chip_8 = chip8_create();
    device->display = display_create("CHIP-8 emulator", SCREEN_WIDTH * 10, SCREEN_HEIGHT * 10, args->bg_color, args->fg_color);
    device->beeper = beeper_create(args->tone);
    device->rom_path = args->rom_path;
    device->ipf = args->ipf;
    device->t1 = device->t60 = SDL_GetTicks();
    device->frames = 0;
    device->running = 1;
    return device;
}

void device_destroy(device_t **device)
{
    display_destroy(&(*device)->display);
    beeper_destroy(&(*device)->beeper);
    chip8_destroy(&(*device)->chip_8);
    free(*device);
    *device = NULL;
}

rom_ld_t device_start(device_t *device)
{
    chip8_reset(device->chip_8);
    FILE *rom = fopen(device->rom_path, "rb");
    rom_ld_t status = chip8_load_rom(device->chip_8, rom);
    if (rom) fclose(rom);
    return status;
}

#ifdef __EMSCRIPTEN__
void device_iterate(void *_device) {
    device_t *device = _device;
#else
void device_iterate(device_t *device) {
#endif
    int ticks = SDL_GetTicks();
    input_event_t ie = intput_handle(device->chip_8->KEYBOARD);

    switch (ie) {
        case IE_HALT: device->running = 0; break;
        case IE_RESTART: device_start(device); break;
        case IE_INC_ISP: device->ipf++; break;
        case IE_DEC_ISP: device->ipf -= (device->ipf > 0) ? 1 : 0; break;
    }

    int ms = (device->frames % 3 == 0) ? 16 : 17;
#ifndef __EMSCRIPTEN__
    if (ticks - device->t60 >= ms) { // ~60 Hz
#endif
        chip8_tick(device->chip_8);

        for (int i = 0; i < device->ipf; i++) {
            chip8_cycle(device->chip_8);
        }

        if (device->chip_8->RF) {
            display_render(device->display, (uint8_t *)device->chip_8->SCREEN, SCREEN_WIDTH, SCREEN_HEIGHT, 10);
            device->chip_8->RF = 0;
        }
        if (device->chip_8->ST) {
            beeper_beep(device->beeper);
        } else {
            beeper_mute(device->beeper);
        }

        device->frames++;
#ifndef __EMSCRIPTEN__
        device->t60 = SDL_GetTicks();
        SDL_Delay(10);
    }
#endif

    if (ticks - device->t1 >= 1000) { // 1 Hz
        char buffer[TITLE_LENGTH];
        snprintf(buffer, TITLE_LENGTH, "CHIP-8 Emulator (%d FPS; %d IPS) - %s", device->frames, device->ipf * device->frames, device->rom_path);
        display_title_set(device->display, buffer);
        device->t1 = SDL_GetTicks();
        device->frames = 0;
    }
}

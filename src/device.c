#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <SDL2/SDL.h>
#include "device.h"
#include "chip8.h"
#include "input.h"
#include "display.h"

device_t *device_init(char *rom_path, uint16_t ipf)
{
    srand(time(NULL));
    device_t *device = malloc(sizeof(device_t));
    device->chip_8 = chip8_create();
    device->display = display_create("CHIP-8 emulator", SCREEN_WIDTH * 10, SCREEN_HEIGHT * 10);
    device->rom_path = rom_path;
    device->ipf = ipf;
    device->t1 = device->t60 = SDL_GetTicks();
    device->frames = 0;
    device->running = 1;
    return device;
}

void device_destroy(device_t **device)
{
    display_destroy(&(*device)->display);
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

void device_iterate(device_t *device)
{
    int ticks = SDL_GetTicks();
    input_event_t ie = intput_handle(device->chip_8->KEYBOARD);

    switch (ie) {
        case IE_HALT: device->running = 0; break;
        case IE_RESTART: device_start(device); break;
        case IE_INC_ISP: device->ipf++; break;
        case IE_DEC_ISP: device->ipf -= (device->ipf > 0) ? 1 : 0; break;
    }

    int ms = (device->frames % 3 == 0) ? 16 : 17;

    if (ticks - device->t60 >= ms) { // ~60 Hz
        chip8_tick(device->chip_8);

        for (int i = 0; i < device->ipf; i++) {
            chip8_cycle(device->chip_8);
        }

        if (device->chip_8->RF) {
            display_render(device->display, (uint8_t *)device->chip_8->SCREEN, SCREEN_WIDTH, SCREEN_HEIGHT, 10);
            device->chip_8->RF = 0;
        }

        device->frames++;
        device->t60 = SDL_GetTicks();
        SDL_Delay(10);
    }

    if (ticks - device->t1 >= 1000) { // 1 Hz
        char *title;
        sprintf(title, "CHIP-8 emulator (%i FPS; %i IPS) - %s", device->frames, device->ipf * device->frames, device->rom_path);
        SDL_SetWindowTitle(device->display->window, title);
        device->t1 = SDL_GetTicks();
        device->frames = 0;
    }
}

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#include "chip8.h"

#define FPS 60

int main(int argc, char *argv[])
{
    if (argc < 2) {
        return EXIT_FAILURE;
    }

    srand(time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        return EXIT_FAILURE;
    SDL_Window *window = SDL_CreateWindow("CHIP-8 emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 320, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        return EXIT_FAILURE;
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        SDL_DestroyWindow(window);
        return EXIT_FAILURE;
    }

    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    
    FILE *rom = fopen(argv[1], "rb");
    rom_ld_t status = chip8_load_rom(c8, rom);
    if (rom) fclose(rom);

    if (status != ROM_LOAD_SUCCESS) {
        return EXIT_FAILURE;
    }

    int IPS = argc > 2 ? strtol(argv[2], NULL, 10) : 540; // instructions per second, default 540
    int IPF = IPS < FPS ? 1 : IPS / FPS; // instructions per frame, not accurate
    int instructions = 0, frames = 0, elapsed = SDL_GetTicks(); // stats
    int running = 1, delay_ms, tick, tock, delta;
    SDL_Event event;
    SDL_Rect rect;

    while (running) {
        tick = SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE) {
                running = 0;
            }
        }
        for (int i = 0; i < IPF; i++) {
            chip8_cycle(c8);
            instructions++;
        }

        if (c8->RF) {
            for (int x = 0; x < SCREEN_WIDTH; x++) {
                for (int y = 0; y < SCREEN_HEIGHT; y++) {
                    rect.x = x * 10;
                    rect.y = y * 10 ;
                    rect.w = rect.h = 10;
                    SDL_SetRenderDrawColor(renderer, 0, c8->SCREEN[x][y] ? 255 : 0, 0, 255);
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
            SDL_RenderPresent(renderer);
            c8->RF = 0;
        }

        frames++;
        delay_ms = (frames % 3 == 0) ? 16 : 17; // 60 fps on average
        tock = SDL_GetTicks();
        delta = delay_ms - (tock - tick);

        if (tock - elapsed >= 1000) { // print and adjust stats
            printf("elapsed: %d ms, instructions: %d, frames: %d\n", tock - elapsed, instructions, frames);
            elapsed = tock;
            instructions = 0;
            frames = 0;
        }
        SDL_Delay(delta < 0 ? 0 : delta); // prevent underflow
    }

    chip8_destroy(&c8);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}

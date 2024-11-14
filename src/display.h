#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL2/SDL.h>

typedef struct display_t {
    SDL_Window *window;
    SDL_Renderer *renderer;
} display_t;

display_t *display_create(char* title, int width, int height);
void display_render(display_t *display, uint8_t *screen_buffer, int width, int height, int pixel_size);
void display_destroy(display_t **display);

#endif

#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL2/SDL.h>

typedef struct color_t {
    uint8_t R;
    uint8_t G;
    uint8_t B;
} color_t;

typedef struct display_t {
    SDL_Window *window;
    SDL_Renderer *renderer;
    color_t bg_color;
    color_t fg_color;
} display_t;

display_t *display_create(char* title, int width, int height, uint32_t bg_color, uint32_t fg_color);
void display_render(display_t *display, uint8_t *screen_buffer, int width, int height, int pixel_size);
void display_destroy(display_t **display);

#endif

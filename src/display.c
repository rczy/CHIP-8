#include <stdlib.h>
#include "display.h"

display_t *display_create(char* title, int width, int height)
{
    SDL_Window *window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        return NULL;
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        SDL_DestroyWindow(window);
        return NULL;
    }
    display_t *display = malloc(sizeof(display_t));
    display->window = window;
    display->renderer = renderer;
    return display;
}

void display_render(display_t *display, uint8_t *screen_buffer, int width, int height, int pixel_size)
{
    SDL_Rect rect;
    SDL_SetRenderDrawColor(display->renderer, 0, 0, 0, 255);
    SDL_RenderClear(display->renderer);
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            rect.x = x * pixel_size;
            rect.y = y * pixel_size ;
            rect.w = rect.h = pixel_size;
            SDL_SetRenderDrawColor(display->renderer, 0, *((screen_buffer + x * height) + y) ? 255 : 0, 0, 255);
            SDL_RenderFillRect(display->renderer, &rect);
        }
    }
    SDL_RenderPresent(display->renderer);
}

void display_destroy(display_t **display)
{
    SDL_DestroyRenderer((*display)->renderer);
    SDL_DestroyWindow((*display)->window);
    free(*display);
    *display = NULL;
}

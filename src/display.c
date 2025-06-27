#include <stdlib.h>
#include "include/display.h"

display_t *display_create(char* title, int width, int height, uint32_t bg_color, uint32_t fg_color)
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
    display->bg_color = (color_t) {
        .R = bg_color >> 16,
        .G = (bg_color & 0x00FF00) >> 8,
        .B = bg_color & 0x0000FF,
    };
    display->fg_color = (color_t) {
        .R = fg_color >> 16,
        .G = (fg_color & 0x00FF00) >> 8,
        .B = fg_color & 0x0000FF,
    };
    return display;
}

void display_render(display_t *display, uint8_t *screen_buffer, int width, int height, int pixel_size)
{
    SDL_Rect rect;
    uint8_t pixel_set;
    color_t *color;

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            rect.x = x * pixel_size;
            rect.y = y * pixel_size ;
            rect.w = rect.h = pixel_size;
            pixel_set = *((screen_buffer + x * height) + y);
            color = (pixel_set) ? &display->fg_color : &display->bg_color;
            SDL_SetRenderDrawColor(display->renderer, color->R, color->G, color->B, 255);
            SDL_RenderFillRect(display->renderer, &rect);
        }
    }
    SDL_RenderPresent(display->renderer);
}

void display_title_set(display_t* display, char* title)
{
    SDL_SetWindowTitle(display->window, title);
}

void display_destroy(display_t **display)
{
    SDL_DestroyRenderer((*display)->renderer);
    SDL_DestroyWindow((*display)->window);
    free(*display);
    *display = NULL;
}

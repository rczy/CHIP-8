#include <SDL2/SDL.h>
#include "include/input.h"

SDL_Keycode key_mapping[16] = {
    SDL_SCANCODE_X, // 0
    SDL_SCANCODE_1, // 1
    SDL_SCANCODE_2, // 2
    SDL_SCANCODE_3, // 3
    SDL_SCANCODE_Q, // 4
    SDL_SCANCODE_W, // 5
    SDL_SCANCODE_E, // 6
    SDL_SCANCODE_A, // 7
    SDL_SCANCODE_S, // 8
    SDL_SCANCODE_D, // 9
    SDL_SCANCODE_Z, // A
    SDL_SCANCODE_C, // B
    SDL_SCANCODE_4, // C
    SDL_SCANCODE_R, // D
    SDL_SCANCODE_F, // E
    SDL_SCANCODE_V, // F
};

input_event_t intput_handle(uint8_t *c8_keyboard)
{
    input_event_t input_event = IE_NONE;
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT: {
                input_event = IE_HALT;
                break;
            }
            case SDL_WINDOWEVENT: {
                if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
                    input_event = IE_HALT;
                }
                break;
            }
            case SDL_KEYDOWN:
            case SDL_KEYUP: {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE: {
                        input_event = IE_HALT;
                        break;
                    }
                    case SDLK_BACKSPACE: {
                        input_event = IE_RESTART;
                        break;
                    }
                    case SDLK_KP_PLUS: {
                        input_event = IE_INC_ISP;
                        break;
                    }
                    case SDLK_KP_MINUS: {
                        input_event = IE_DEC_ISP;
                        break;
                    }
                    default: {
                        for (int i = 0; i < 16; i++) {
                            if (event.key.keysym.scancode == key_mapping[i]) {
                                c8_keyboard[i] = event.type == SDL_KEYDOWN;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    return input_event;
}

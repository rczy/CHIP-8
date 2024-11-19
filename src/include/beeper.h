#ifndef BEEPER_H
#define BEEPER_H

#include <stdint.h>
#include <SDL2/SDL.h>

#define SAMPLE_FREQ 44100
#define SAMPLES 2048

typedef enum beeper_state_t { BEEPING, MUTED } beeper_state_t;

typedef struct beeper_t {
    SDL_AudioDeviceID id;
    beeper_state_t state;
    unsigned int period;
    unsigned int half_period;
    unsigned int nth_chunk;
} beeper_t;

beeper_t *beeper_create(uint16_t tone);
void beeper_destroy(beeper_t **beeper);
void beeper_beep(beeper_t *beeper);
void beeper_mute(beeper_t *beeper);

#endif

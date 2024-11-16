#ifndef BEEPER_H
#define BEEPER_H

#include <stdint.h>
#include <SDL2/SDL.h>

#define SAMPLE_FREQ 44100
#define SAMPLES 2048
#define BUFFER_LENGTH SAMPLE_FREQ
#define TONE 440

typedef enum beeper_state_t { BEEPING, MUTED } beeper_state_t;

typedef struct beeper_t {
    SDL_AudioDeviceID id;
    SDL_AudioSpec *spec;
    beeper_state_t state;
    unsigned int buffer_pos;
    int16_t buffer[BUFFER_LENGTH];
} beeper_t;

beeper_t *beeper_create();
void beeper_destroy(beeper_t **beeper);
void beeper_beep(beeper_t *beeper);
void beeper_mute(beeper_t *beeper);

#endif

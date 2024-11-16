#include "include/beeper.h"

void beeper_callback(void *userdata, uint8_t * stream, int len);
void beeper_buffer_init(beeper_t *beeper);

beeper_t *beeper_create()
{
    beeper_t *beeper = malloc(sizeof(beeper_t));
    beeper->state = MUTED;
    beeper_buffer_init(beeper);
    beeper->buffer_pos = 0;

    SDL_AudioSpec *spec = malloc(sizeof(SDL_AudioSpec));
    spec->freq = SAMPLE_FREQ;
    spec->format = AUDIO_S16SYS;
    spec->channels = 1;
    spec->samples = SAMPLES;
    spec->callback = beeper_callback;
    spec->userdata = beeper;
    beeper->spec = spec;

    beeper->id = SDL_OpenAudioDevice(NULL, 0, spec, NULL, 0);

    return beeper->id ? beeper : NULL;
}

void beeper_destroy(beeper_t **beeper)
{
    free((*beeper)->spec);
    SDL_CloseAudioDevice((*beeper)->id);
    free(*beeper);
    *beeper = NULL;
}

void beeper_beep(beeper_t *beeper)
{
    if (beeper->state == BEEPING) {
        return;
    }
    SDL_PauseAudioDevice(beeper->id, BEEPING);
    beeper->state = BEEPING;
}

void beeper_mute(beeper_t *beeper)
{
    if (beeper->state == MUTED) {
        return;
    }
    SDL_PauseAudioDevice(beeper->id, MUTED);
    beeper->state = MUTED;
}

void beeper_buffer_init(beeper_t *beeper)
{
    int period = SAMPLE_FREQ / TONE;

    for (int i = 0; i < SAMPLE_FREQ; i++) {
        beeper->buffer[i] = (i % period < period / 2) ? INT16_MIN : INT16_MAX;
    }
}

void beeper_callback(void *userdata, uint8_t *stream, int len)
{
    beeper_t *beeper = (beeper_t *)userdata;
    int16_t *out = (int16_t *)stream;
    for (int i = 0; i < len / 2; i++) {
        out[i] = beeper->buffer[beeper->buffer_pos++];
        if (beeper->buffer_pos == BUFFER_LENGTH) {
            beeper->buffer_pos = 0;
        }
    }
}

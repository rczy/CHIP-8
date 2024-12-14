#include "include/beeper.h"

void beeper_callback(void *userdata, uint8_t * stream, int len);

beeper_t *beeper_create(uint16_t tone)
{
    if (tone == 0) {
        tone = SAMPLE_FREQ;
    }

    beeper_t *beeper = malloc(sizeof(beeper_t));
    beeper->state = MUTED;
    beeper->period = SAMPLE_FREQ / tone;
    beeper->half_period = beeper->period / 2;
    beeper->nth_chunk = 0;

    SDL_AudioSpec spec = {
        .freq = SAMPLE_FREQ,
        .format = AUDIO_S16SYS,
        .channels = 1,
        .samples = SAMPLES,
        .callback = beeper_callback,
        .userdata = beeper,
    };
    beeper->id = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);

    return beeper->id ? beeper : NULL;
}

void beeper_destroy(beeper_t **beeper)
{
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

void beeper_callback(void *userdata, uint8_t *stream, int len)
{
    beeper_t *beeper = (beeper_t *)userdata;
    int16_t *out = (int16_t *)stream;
    for (int i = 0; i < len / 2; i++) {
        out[i] = (beeper->nth_chunk++ % beeper->period < beeper->half_period) ? INT16_MIN : INT16_MAX;
        if (beeper->nth_chunk == beeper->period) {
            beeper->nth_chunk = 0;
        }
    }
}

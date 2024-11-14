#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>

typedef enum input_event_t { IE_NONE, IE_HALT = 1, IE_RESTART = 2, IE_INC_ISP = 4, IE_DEC_ISP = 8 } input_event_t;

input_event_t intput_handle(uint8_t *c8_keyboard);

#endif

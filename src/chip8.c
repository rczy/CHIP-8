#include <stdlib.h>
#include <string.h>

#include "chip8.h"

chip8_t *chip8_create()
{
    return malloc(sizeof(chip8_t));
}

void chip8_destroy(chip8_t **c8)
{
    free(*c8);
    *c8 = NULL;
}

void chip8_reset(chip8_t *c8)
{
    memset(c8->RAM, 0, sizeof(uint8_t) * RAM_SIZE);
    memset(c8->V, 0, sizeof(uint8_t) * 16);
    memset(c8->S, 0, sizeof(uint16_t) * STACK_SIZE);
    c8->I = c8->OP = c8->SP = 0;
    c8->DT = c8->ST = 0;
    c8->PC = START_ADDRESS;
}

void chip8_ramcpy(chip8_t *c8, uint8_t *bytes, uint8_t size)
{
    memcpy(&c8->RAM[START_ADDRESS], bytes, sizeof(uint8_t) * size);
}

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
    c8->I = c8->SP = 0;
    c8->DT = c8->ST = 0;
    c8->PC = START_ADDRESS;
}

void chip8_ramcpy(chip8_t *c8, uint8_t *bytes, uint8_t size)
{
    memcpy(&c8->RAM[START_ADDRESS], bytes, sizeof(uint8_t) * size);
}

exec_res_t chip8_cycle(chip8_t *c8)
{
    uint16_t opcode = chip8_fetch(c8);
    instruction_t inst;
    chip8_decode(opcode, &inst);
    return chip8_execute(c8, &inst);
}

uint16_t chip8_fetch(chip8_t *c8)
{
    uint8_t OH, OL;
    OH = c8->RAM[c8->PC];
    OL = c8->RAM[c8->PC + 1];
    c8->PC += 2;
    return OH << 8 | OL;
}

void chip8_decode(uint16_t opcode, instruction_t *inst)
{
    inst->OP = opcode;
    inst->X = (opcode & 0x0F00) >> 8;
    inst->Y = (opcode & 0x00F0) >> 4;
    inst->N = opcode & 0x000F;
    inst->NN = opcode & 0x00FF;
    inst->NNN = opcode & 0x0FFF;
}

exec_res_t chip8_execute(chip8_t *c8, instruction_t *inst)
{
    switch (inst->OP & 0xF000) {
        default: return UNKNOWN_OPCODE;
    }
    return EXEC_SUCCESS;
}

#include <stdio.h>
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
    memset(c8->STACK, 0, sizeof(uint16_t) * STACK_SIZE);
    memset(c8->SCREEN, 0, sizeof(uint8_t) * SCREEN_WIDTH * SCREEN_HEIGHT);
    memset(c8->KEYBOARD, 0, sizeof(uint8_t) * 16);
    c8->I = c8->SP = c8->RF = 0;
    c8->DT = c8->ST = 0;
    c8->PC = START_ADDRESS;
}

void chip8_ramcpy(chip8_t *c8, uint8_t *bytes, uint8_t size)
{
    memcpy(&c8->RAM[START_ADDRESS], bytes, sizeof(uint8_t) * size);
}

rom_ld_t chip8_load_rom(chip8_t *c8, FILE *rom)
{
    if (rom == NULL) {
        return ROM_NOT_EXISTS;
    }
    fseek(rom, 0, SEEK_END);
    size_t bytes = ftell(rom);
    rewind(rom);
    if (bytes > RAM_SIZE - START_ADDRESS) {
        return ROM_TOO_LARGE;
    }
    fread(&c8->RAM[START_ADDRESS], sizeof(uint8_t), bytes, rom);
    return ROM_LOAD_SUCCESS;
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
        case 0x0000: {
            switch (inst->OP) {
                case 0x0000: { // no operation
                    break;
                }
                case 0x00E0: { // clear screen
                    memset(c8->SCREEN, 0, sizeof(uint8_t) * SCREEN_WIDTH * SCREEN_HEIGHT);
                    c8->RF = 1;
                    break;
                }
                case 0x00EE: { // return from subroutine
                    if (c8->SP == 0) return STACK_UNDERFLOW;
                    c8->SP--;
                    c8->PC = c8->STACK[c8->SP];
                    break;
                }
                default: return UNKNOWN_OPCODE;
            }
            break;
        }
        case 0x1000: { // jump to address NNN
            c8->PC = inst->NNN;
            break;
        }
        case 0x2000: { // call subroutine
            if (c8->SP == STACK_SIZE) return STACK_OVERFLOW;
            c8->STACK[c8->SP] = c8->PC;
            c8->SP++;
            c8->PC = inst->NNN;
            break;
        }
        case 0x3000: { // skip if VX == NN
            if (c8->V[inst->X] == inst->NN) {
                c8->PC += 2;
            }
            break;
        }
        case 0x4000: { // skip if VX != NN
            if (c8->V[inst->X] != inst->NN) {
                c8->PC += 2;
            }
            break;
        }
        case 0x5000: { // skip if VX == VY
            if (inst->N != 0) return UNKNOWN_OPCODE;
            if (c8->V[inst->X] == c8->V[inst->Y]) {
                c8->PC += 2;
            }
            break;
        }
        case 0x6000: { // VX = NN
            c8->V[inst->X] = inst->NN;
            break;
        }
        case 0x7000: { // VX += NN
            c8->V[inst->X] += inst->NN;
            break;
        }
        case 0x8000: {
            switch (inst->N) {
                case 0: { // VX = VY
                    c8->V[inst->X] = c8->V[inst->Y];
                    break;
                }
                case 1: { // VX |= VY
                    c8->V[inst->X] |= c8->V[inst->Y];
                    break;
                }
                case 2: { // VX &= VY
                    c8->V[inst->X] &= c8->V[inst->Y];
                    break;
                }
                case 3: { // VX ^= VY
                    c8->V[inst->X] ^= c8->V[inst->Y];
                    break;
                }
                case 4: { // VX += VY
                    int result = c8->V[inst->X] + c8->V[inst->Y];
                    c8->V[inst->X] = result;
                    c8->V[0xF] = result > 255;
                    break;
                }
                case 5: { // VX -= VY
                    c8->V[0xF] = c8->V[inst->X] > c8->V[inst->Y];
                    c8->V[inst->X] -= c8->V[inst->Y];
                    break;
                }
                case 6: { // VX = VY >> 1
                    c8->V[0xF] = c8->V[inst->X] & 1;
                    c8->V[inst->X] >>= 1;
                    break;
                }
                case 7: { // VX = VY - VX
                    c8->V[0xF] = c8->V[inst->Y] > c8->V[inst->X];
                    c8->V[inst->X] = c8->V[inst->Y] - c8->V[inst->X];
                    break;
                }
                case 0xE: { // VX = VY << 1
                    c8->V[0xF] = c8->V[inst->X] >> 7;
                    c8->V[inst->X] <<= 1;
                    break;
                }
                default: return UNKNOWN_OPCODE;
            }
            break;
        }
        case 0x9000: { // skip if VX != VY
            if (inst->N != 0) return UNKNOWN_OPCODE;
            if (c8->V[inst->X] != c8->V[inst->Y]) {
                c8->PC += 2;
            }
            break;
        }
        case 0xA000: { // I = NNN
            c8->I = inst->NNN;
            break;
        }
        case 0xB000: { // jump to address NNN + V0
            c8->PC = inst->NNN + c8->V[0];
            break;
        }
        case 0xC000: { // VX = random NN
            c8->V[inst->X] = (rand() % 256) & inst->NN;
            break;
        }
        case 0xD000: { // draw
            c8->V[0xF] = 0;
            uint8_t X = c8->V[inst->X] % SCREEN_WIDTH;
            uint8_t Y = c8->V[inst->Y] % SCREEN_HEIGHT;
            for (int py = 0; py < inst->N && py + Y < SCREEN_HEIGHT; py++) {
                uint8_t pattern = c8->RAM[c8->I + py];
                for (int px = 0; px <= 7 && px + X < SCREEN_WIDTH; px++) {
                    uint8_t pixel = (pattern >> (7 - px)) & 1;
                    if (pixel) {
                        c8->V[0xF] |= c8->SCREEN[px + X][py + Y];
                        c8->SCREEN[px + X][py + Y] ^= pixel;
                    }
                }
            }
            c8->RF = 1;
            break;
        }
        default: return UNKNOWN_OPCODE;
    }
    if (c8->PC >= RAM_SIZE) {
        c8->PC = START_ADDRESS;
        return PC_OVERFLOW;
    }
    return EXEC_SUCCESS;
}

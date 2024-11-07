#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>

#define RAM_SIZE 4096
#define STACK_SIZE 64

#define START_ADDRESS 0x200

typedef struct chip8_t {
    uint8_t RAM[RAM_SIZE];  // RAM
    uint8_t V[16];          // V0-VF register
    uint16_t I;             // index register
    uint16_t OP;            // opcode
    uint16_t S[STACK_SIZE]; // stack
    uint16_t SP;            // stack pointer
    uint16_t PC;            // program counter
    uint8_t DT;             // delay timer
    uint8_t ST;             // sound timer
} chip8_t;

chip8_t *chip8_create();
void chip8_destroy(chip8_t **c8);
void chip8_reset(chip8_t *c8);
void chip8_ramcpy(chip8_t *c8, uint8_t *bytes, uint8_t size);
void chip8_fetch(chip8_t *c8);

#endif

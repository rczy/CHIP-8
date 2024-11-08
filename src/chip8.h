#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>

#define RAM_SIZE 4096
#define STACK_SIZE 16

#define START_ADDRESS 0x200

typedef struct chip8_t {
    uint8_t RAM[RAM_SIZE];  // RAM
    uint8_t V[16];          // V0-VF register
    uint16_t I;             // index register
    uint16_t S[STACK_SIZE]; // stack
    uint8_t SP;             // stack pointer
    uint16_t PC;            // program counter
    uint8_t DT;             // delay timer
    uint8_t ST;             // sound timer
} chip8_t;

typedef struct instruction_t {
    uint16_t OP;    // opcode
    uint8_t X;      // X index for V register
    uint8_t Y;      // Y index for V register
    uint8_t N;      // 4th nibble
    uint8_t NN;     // 3rd and 4th nibble
    uint16_t NNN;   // 2nd, 3rd and 4th nibble
} instruction_t;

typedef enum exec_res_t { EXEC_SUCCESS, UNKNOWN_OPCODE, STACK_OVERFLOW } exec_res_t;

chip8_t *chip8_create();
void chip8_destroy(chip8_t **c8);
void chip8_reset(chip8_t *c8);
void chip8_ramcpy(chip8_t *c8, uint8_t *bytes, uint8_t size);
uint16_t chip8_fetch(chip8_t *c8);
void chip8_decode(uint16_t opcode, instruction_t *inst);
exec_res_t chip8_execute(chip8_t *c8, instruction_t *inst);
exec_res_t chip8_cycle(chip8_t *c8);

#endif

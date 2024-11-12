#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stdio.h>

#define RAM_SIZE 4096
#define STACK_SIZE 16

#define START_ADDRESS 0x200

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

#define FONTSET_ADDRESS 0x100
#define FONT_OFFSET 5

typedef struct chip8_t {
    // RAM
    uint8_t RAM[RAM_SIZE];
    // V0-VF register
    uint8_t V[16];
    // index register
    uint16_t I;
    // stack
    uint16_t STACK[STACK_SIZE];
    // stack pointer
    uint8_t SP;
    // program counter
    uint16_t PC;
    // delay timer
    uint8_t DT;
    // sound timer
    uint8_t ST;
    // render flag
    uint8_t RF;
    // screen buffer
    uint8_t SCREEN[SCREEN_WIDTH][SCREEN_HEIGHT];
    // keyboard buffer
    uint8_t KEYBOARD[16];
} chip8_t;

typedef struct instruction_t {
    uint16_t OP;    // opcode
    uint8_t X;      // X index for V register
    uint8_t Y;      // Y index for V register
    uint8_t N;      // 4th nibble
    uint8_t NN;     // 3rd and 4th nibble
    uint16_t NNN;   // 2nd, 3rd and 4th nibble
} instruction_t;

typedef enum exec_res_t { EXEC_SUCCESS, UNKNOWN_OPCODE, STACK_OVERFLOW, STACK_UNDERFLOW, PC_OVERFLOW } exec_res_t;

typedef enum rom_ld_t { ROM_LOAD_SUCCESS, ROM_NOT_EXISTS, ROM_TOO_LARGE } rom_ld_t;

chip8_t *chip8_create();
void chip8_destroy(chip8_t **c8);
void chip8_reset(chip8_t *c8);
void chip8_ramcpy(chip8_t *c8, uint8_t *bytes, uint8_t size);
rom_ld_t chip8_load_rom(chip8_t *c8, FILE *f);
uint16_t chip8_fetch(chip8_t *c8);
void chip8_decode(uint16_t opcode, instruction_t *inst);
exec_res_t chip8_execute(chip8_t *c8, instruction_t *inst);
exec_res_t chip8_cycle(chip8_t *c8);
void chip8_tick(chip8_t *c8);

#endif

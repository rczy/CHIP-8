/**
 * Tests for instruction executing.
 */

#include "../lib/acutest.h"
#include "../src/chip8.c"

/**
 * PC overflow
 */
void test_PC_overflow(void)
{
    instruction_t inst;
    exec_res_t result;
    uint16_t opcode;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    c8->PC = RAM_SIZE - 4;
    // cycle 1
    opcode = chip8_fetch(c8);
    TEST_CHECK(opcode == 0);
    chip8_decode(opcode, &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->PC == RAM_SIZE - 2);
    // cycle 2
    opcode = chip8_fetch(c8);
    TEST_CHECK(c8->PC == RAM_SIZE);
    TEST_CHECK(opcode == 0);
    chip8_decode(opcode, &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == PC_OVERFLOW);
    TEST_CHECK(c8->PC == START_ADDRESS);
    chip8_destroy(&c8);
}

/**
 * unknown opcode - 0xFFFF
 */
void test_unknown_opcode(void)
{
    uint8_t data[] = { 0xFF, 0xFF };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == UNKNOWN_OPCODE);
    chip8_destroy(&c8);
}

/**
 * no operation
 */
void test_0x0000(void)
{
    uint8_t data[] = { 0x00, 0x00 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->I == 0);
    TEST_CHECK(c8->SP == 0);
    TEST_CHECK(c8->PC == START_ADDRESS + 2);
    TEST_CHECK(c8->DT == 0);
    TEST_CHECK(c8->ST == 0);
    TEST_CHECK(c8->RF == 0);
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += c8->V[i];
    }
    TEST_CHECK(sum == 0);
    for (int i = 0; i < STACK_SIZE; i++) {
        sum += c8->STACK[i];
    }
    TEST_CHECK(sum == 0);
    for (int i = START_ADDRESS; i < RAM_SIZE; i++) {
        sum += c8->RAM[i];
    }
    TEST_CHECK(sum == 0);
    for (int i = 0; i < SCREEN_WIDTH; i++) {
        for (int j = 0; j < SCREEN_HEIGHT; j++) {
            sum += c8->SCREEN[i][j];
        }
    }
    TEST_CHECK(sum == 0);
    for (int i = 0; i < 16; i++) {
        sum += c8->KEYBOARD[i];
    }
    TEST_CHECK(sum == 0);
    chip8_destroy(&c8);
}

/**
 * clear screen
 */
void test_0x00E0(void)
{
    uint8_t data[] = { 0x00, 0xE0 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);

    memset(c8->SCREEN, 1, sizeof(uint8_t) * SCREEN_WIDTH * SCREEN_HEIGHT);

    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->RF == 1);
    int sum = 0;
    for (int i = 0; i < SCREEN_WIDTH; i++) {
        for (int j = 0; j < SCREEN_HEIGHT; j++) {
            sum += c8->SCREEN[i][j];
        }
    }
    TEST_CHECK(sum == 0);
    chip8_destroy(&c8);
}

/**
 * return from subroutine
 */
void test_0x00EE(void)
{
    uint8_t data[] = { 0x00, 0xEE };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);

    TEST_CHECK(c8->SP == 0);
    TEST_CHECK(c8->STACK[c8->SP] != START_ADDRESS);
    c8->STACK[c8->SP] = START_ADDRESS;
    c8->SP++;

    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->SP == 0);
    TEST_CHECK(c8->PC == c8->STACK[c8->SP]);
    TEST_CHECK(c8->PC == START_ADDRESS);
    chip8_destroy(&c8);
}

/**
 * return from subroutine, stack underflow
 */
void test_0x00EE_stack_underflow(void)
{
    uint8_t data[] = { 0x00, 0xEE };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);

    TEST_CHECK(c8->SP == 0);

    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == STACK_UNDERFLOW);
    chip8_destroy(&c8);
}

/**
 * jump to address NNN
 */
void test_0x1NNN(void)
{
    uint8_t data[] = { 0x12, 0xC8 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);

    c8->RAM[0x2C8] = 0xFF;
    c8->RAM[0x2C9] = 0xFF;

    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(inst.NNN == 0x2C8);
    TEST_CHECK(c8->PC == 0x2C8);
    TEST_CHECK(chip8_fetch(c8) == 0xFFFF);
    chip8_destroy(&c8);
}

/**
 * call subroutine
 */
void test_0x2NNN(void)
{
    uint8_t data[] = { 0x21, 0xC8 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);

    TEST_CHECK(c8->SP == 0);
    TEST_CHECK(c8->STACK[c8->SP] != START_ADDRESS);

    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->SP == 1);
    TEST_CHECK(c8->STACK[c8->SP - 1] == START_ADDRESS + 2);
    TEST_CHECK(c8->PC == 0x1C8);
    chip8_destroy(&c8);
}

/**
 * call subroutine, stack overflow
 */
void test_0x2NNN_stack_overflow(void)
{
    uint8_t data[] = { 0x21, 0xC8 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);

    c8->SP = STACK_SIZE; // stack is full

    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == STACK_OVERFLOW);
    chip8_destroy(&c8);
}

/**
 * skip if VX == NN
 */
void test_0x3XNN(void)
{
    uint8_t data[] = { 0x30, 0xC8, 0x12, 0x34, 0x56, 0x78 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 6);

    // VX != NN, no skip
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(chip8_fetch(c8) == 0x1234);

    // VX == NN, skip
    c8->PC = START_ADDRESS;
    c8->V[0] = 0xC8;
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(chip8_fetch(c8) == 0x5678);

    chip8_destroy(&c8);
}

/**
 * skip if VX != NN
 */
void test_0x4XNN(void)
{
    uint8_t data[] = { 0x40, 0xC8, 0x12, 0x34, 0x56, 0x78 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 6);

    // VX != NN, skip
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(chip8_fetch(c8) == 0x5678);

    // VX == NN, no skip
    c8->PC = START_ADDRESS;
    c8->V[0] = 0xC8;
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(chip8_fetch(c8) == 0x1234);

    chip8_destroy(&c8);
}

/**
 * skip if VX == VY
 */
void test_0x5XY0(void)
{
    uint8_t data[] = { 0x5A, 0xB0, 0x12, 0x34, 0x56, 0x78 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 6);

    // VX == VY, skip
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(chip8_fetch(c8) == 0x5678);

    // VX != VY, no skip
    c8->PC = START_ADDRESS;
    c8->V[0xA] = 0xC8;
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(chip8_fetch(c8) == 0x1234);

    // opcode ends with non-zero
    c8->RAM[START_ADDRESS + 1] = 0xB1;
    c8->PC = START_ADDRESS;
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == UNKNOWN_OPCODE);

    chip8_destroy(&c8);
}

/**
 * skip if VX != VY
 */
void test_0x9XY0(void)
{
    uint8_t data[] = { 0x9A, 0xB0, 0x12, 0x34, 0x56, 0x78 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 6);

    // VX == VY, no skip
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(chip8_fetch(c8) == 0x1234);

    // VX != VY, skip
    c8->PC = START_ADDRESS;
    c8->V[0xA] = 0xC8;
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(chip8_fetch(c8) == 0x5678);

    // opcode ends with non-zero
    c8->RAM[START_ADDRESS + 1] = 0xB1;
    c8->PC = START_ADDRESS;
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == UNKNOWN_OPCODE);

    chip8_destroy(&c8);
}

/**
 * VX = NN
 */
void test_0x6XNN(void)
{
    uint8_t data[] = { 0x6A, 0x12 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);

    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->V[0xA] == 0x12);

    chip8_destroy(&c8);
}

/**
 * VX += NN
 */
void test_0x7XNN(void)
{
    uint8_t data[] = { 0x7A, 0x12 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);
    c8->V[0xA] = 0xE; // 14

    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->V[0xA] == 0x20); // 14 + 18 = 32

    // check overflow
    c8->V[0xA] = 0xEE; // 238
    c8->PC = START_ADDRESS;
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->V[0xA] == 0); // 238 + 18 = 256, overflow

    chip8_destroy(&c8);
}

/**
 * 0x8XYA - unknown opcode
 */
void test_0x8XYA(void)
{
    uint8_t data[] = { 0x8A, 0x1A };
    instruction_t inst;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);
    chip8_decode(chip8_fetch(c8), &inst);
    TEST_CHECK(chip8_execute(c8, &inst) == UNKNOWN_OPCODE);
    chip8_destroy(&c8);
}

/**
 * VX = VY
 */
void test_0x8XY0(void)
{
    uint8_t data[] = { 0x8A, 0xB0 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);

    c8->V[0xB] = 0xC;
    TEST_CHECK(c8->V[0xA] == 0);
    
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->V[0xA] == c8->V[0xB]);
    chip8_destroy(&c8);
}

/**
 * VX |= VY
 */
void test_0x8XY1(void)
{
    uint8_t data[] = { 0x8A, 0xB1 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);

    c8->V[0xA] = 0b0011;
    c8->V[0xB] = 0b0101;
    
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->V[0xA] == 0b0111);
    chip8_destroy(&c8);
}

/**
 * VX &= VY
 */
void test_0x8XY2(void)
{
    uint8_t data[] = { 0x8A, 0xB2 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);

    c8->V[0xA] = 0b0011;
    c8->V[0xB] = 0b0101;
    
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->V[0xA] == 0b0001);
    chip8_destroy(&c8);
}

/**
 * VX ^= VY
 */
void test_0x8XY3(void)
{
    uint8_t data[] = { 0x8A, 0xB3 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);

    c8->V[0xA] = 0b0011;
    c8->V[0xB] = 0b0101;
    
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->V[0xA] == 0b0110);
    chip8_destroy(&c8);
}

/**
 * VX += VY
 */
void test_0x8XY4(void)
{
    uint8_t data[] = { 0x8A, 0xB4 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);
    
    c8->V[0xA] = 0x0E; // 14
    c8->V[0xB] = 0x12; // 18

    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->V[0xA] == 0x20); // 14 + 18 = 32
    TEST_CHECK(c8->V[0xB] == 0x12); // unchanged
    TEST_CHECK(c8->V[0xF] == 0); // no overflow

    // check overflow
    c8->V[0xA] = 0xF0; // 240
    c8->PC = START_ADDRESS;
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->V[0xA] == 0x02); // 240 + 18 = 258, overflow
    TEST_CHECK(c8->V[0xB] == 0x12); // unchanged
    TEST_CHECK(c8->V[0xF] == 1); // overflow

    chip8_destroy(&c8);
}

/**
 * VX -= VY
 */
void test_0x8XY5(void)
{
    uint8_t data[] = { 0x8A, 0xB5 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);

    c8->V[0xA] = 0x12; // 18
    c8->V[0xB] = 0x0E; // 14

    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->V[0xA] == 0x04); // 18 - 14 = 0
    TEST_CHECK(c8->V[0xB] == 0x0E); // unchanged
    TEST_CHECK(c8->V[0xF] == 1); // VX > VY ? 1 : 0

    // check underflow
    c8->V[0xA] = 0x0A; // 10
    c8->PC = START_ADDRESS;
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->V[0xA] == 0xFC); // 10 - 14 = -4, underflow
    TEST_CHECK(c8->V[0xB] == 0x0E); // unchanged
    TEST_CHECK(c8->V[0xF] == 0); // VX > VY ? 1 : 0

    chip8_destroy(&c8);
}

/**
 * VX = VY >> 1
 */
void test_0x8XY6(void)
{
    uint8_t data[] = { 0x8A, 0xB6 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);

    c8->V[0xA] = 0b00001111;
    TEST_CHECK(c8->V[0xF] == 0);

    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->V[0xA] == 0b00000111);
    TEST_CHECK(c8->V[0xF] == 1);

    chip8_destroy(&c8);
}

/**
 * VX = VY - VX
 */
void test_0x8XY7(void)
{
    uint8_t data[] = { 0x8A, 0xB7 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);

    c8->V[0xA] = 0x12; // 18
    c8->V[0xB] = 0x0E; // 14

    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->V[0xA] == 0xFC); // 14 - 18 = -4, underflow
    TEST_CHECK(c8->V[0xB] == 0x0E); // unchanged
    TEST_CHECK(c8->V[0xF] == 0); // VY > VX ? 1 : 0

    // check underflow
    c8->V[0xA] = 0x0A; // 10
    c8->PC = START_ADDRESS;
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->V[0xA] == 0x04); // 14 - 10 = 4
    TEST_CHECK(c8->V[0xB] == 0x0E); // unchanged
    TEST_CHECK(c8->V[0xF] == 1); // VY > VX ? 1 : 0

    chip8_destroy(&c8);
}

/**
 * VX = VY << 1
 */
void test_0x8XYE(void)
{
    uint8_t data[] = { 0x8A, 0xBE };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);

    c8->V[0xA] = 0b10001111;
    TEST_CHECK(c8->V[0xF] == 0);

    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->V[0xA] == 0b00011110);
    TEST_CHECK(c8->V[0xF] == 1);

    chip8_destroy(&c8);
}

/**
 * I = NNN
 */
void test_0xANNN(void)
{
    uint8_t data[] = { 0xA1, 0x23 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);

    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->I == inst.NNN);

    chip8_destroy(&c8);
}

/**
 * jump to address NNN + V0
 */
void test_0xBNNN(void)
{
    uint8_t data[] = { 0xB1, 0x23 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);

    c8->V[0] = 0x12;

    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK_(c8->PC == (0x123 + 0x12), "I: %X", c8->I);

    chip8_destroy(&c8);
}

/**
 * VX = random NN
 */
void test_0xCXNN(void)
{
    srand(time(NULL));
    uint8_t data[] = { 0xCA, 0xC8, 0xCA, 0xC8 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 4);
    TEST_CHECK(c8->V[0xA] == 0);
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->V[0xA] >= 0 && c8->V[0xA] <= 255);
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->V[0xA] >= 0 && c8->V[0xA] <= 255);
    chip8_destroy(&c8);
}

/**
 * draw to (0, 0)
 */
void test_0xDXYN_00(void)
{
    uint8_t data[] = { 0xDA, 0xB6, 0x20, 0x70, 0x70, 0xF8, 0xD8, 0x88 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 8);
    c8->I = 0x202;
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->RF == 1);
    TEST_CHECK(c8->V[0xF] == 0);
    for (int row = 0; row < inst.N; row++) {
        int pattern = 0;
        for (int col = 0; col < 8; col++) {
            pattern |= (c8->SCREEN[col][row] << (7 - col));
        }
        TEST_CHECK(pattern == c8->RAM[(START_ADDRESS + 2) + row]);
    }
    chip8_destroy(&c8);
}

/**
 * draw to (X, Y) and erase
 */
void test_0xDXYN_XY_erase(void)
{
    uint8_t data[] = { 0xDA, 0xB6, 0x20, 0x70, 0x70, 0xF8, 0xD8, 0x88 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 8);
    c8->I = 0x202;
    c8->V[0xA] = 30;
    c8->V[0xB] = 10;
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->RF == 1);
    TEST_CHECK(c8->V[0xF] == 0);
    int X = c8->V[0xA] % SCREEN_WIDTH;
    int Y = c8->V[0xB] % SCREEN_HEIGHT;
    for (int row = 0; row < inst.N && row + Y < SCREEN_HEIGHT; row++) {
        int pattern = 0;
        for (int col = 0; col < 8 && col + X < SCREEN_WIDTH; col++) {
            pattern |= (c8->SCREEN[col + X][row + Y] << (7 - col));
        }
        uint8_t original_pattern = c8->RAM[(START_ADDRESS + 2) + row];
        TEST_CHECK(pattern == original_pattern);
    }
    c8->PC = START_ADDRESS;
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->RF == 1);
    TEST_CHECK(c8->V[0xF] == 1);
    int sum = 0;
    for (int i = 0; i < SCREEN_WIDTH; i++) {
        for (int j = 0; j < SCREEN_HEIGHT; j++) {
            sum += c8->SCREEN[i][j];
        }
    }
    TEST_CHECK(sum == 0);
    chip8_destroy(&c8);
}

/**
 * collision detection
 */
void test_0xDXYN_collision_detection(void)
{
    uint8_t data[] = { 0xDA, 0xB6, 0x20, 0x70, 0x70, 0xF8, 0xD8, 0x88 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 8);
    c8->I = 0x202;
    c8->V[0xA] = 30;
    c8->V[0xB] = 10;
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->RF == 1);
    TEST_CHECK(c8->V[0xF] == 0);
    c8->PC = START_ADDRESS;
    c8->V[0xA] += 4;
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->RF == 1);
    TEST_CHECK(c8->V[0xF] == 1);
    chip8_destroy(&c8);
}

/**
 * draw with wrap and clip
 */
void test_0xDXYN_wrap_and_clip(void)
{
    uint8_t data[] = { 0xDA, 0xB6, 0x20, 0x70, 0x70, 0xF8, 0xD8, 0x88 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 8);
    c8->I = 0x202;
    c8->V[0xA] = 125;
    c8->V[0xB] = 91;
    
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->RF == 1);
    TEST_CHECK(c8->V[0xF] == 0);
    int X = c8->V[0xA] % SCREEN_WIDTH;
    int Y = c8->V[0xB] % SCREEN_HEIGHT;
    for (int row = 0; row < inst.N && row + Y < SCREEN_HEIGHT; row++) {
        uint8_t pattern = 0;
        for (int col = 0; col < 8 && col + X < SCREEN_WIDTH; col++) {
            pattern |= (c8->SCREEN[col + X][row + Y] << (7 - col));
        }
        uint8_t original_pattern = c8->RAM[(START_ADDRESS + 2) + row];
        uint8_t mask = 0xFF << (8 + X - SCREEN_WIDTH);
        original_pattern &= mask;
        TEST_CHECK(pattern == original_pattern);
    }
    chip8_destroy(&c8);
}

/**
 * skip if VX key pressed
 */
void test_0xEX9E(void)
{
    uint8_t data[] = { 0xEA, 0x9E, 0x12, 0x34, 0x56, 0x78 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 6);

    // no skip
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(chip8_fetch(c8) == 0x1234);

    // skip
    c8->PC = START_ADDRESS;
    c8->V[0xA] = 0xD;
    c8->KEYBOARD[0xD] = 1;
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(chip8_fetch(c8) == 0x5678);

    chip8_destroy(&c8);
}

/**
 * skip if VX key not pressed
 */
void test_0xEXA1(void)
{
    uint8_t data[] = { 0xEF, 0xA1, 0x12, 0x34, 0x56, 0x78 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 6);

    // skip
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(chip8_fetch(c8) == 0x5678);

    // no skip
    c8->PC = START_ADDRESS;
    c8->V[0xF] = 0xB;
    c8->KEYBOARD[0xB] = 1;
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(chip8_fetch(c8) == 0x1234);

    chip8_destroy(&c8);
}

/**
 * VX = DT
 */
void test_0xFX07(void)
{
    uint8_t data[] = { 0xF8, 0x07 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);

    c8->DT = 60;
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->V[8] == 60);

    chip8_destroy(&c8);
}

/**
 * wait, set VX = key
 */
void test_0xFX0A(void)
{
    uint8_t data[] = { 0xF2, 0x0A, 0x12, 0x34 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 4);

    // cycle 1
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->V[2] == 0);
    TEST_CHECK(c8->PC = START_ADDRESS);

    // cycle 2
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->V[2] == 0);
    TEST_CHECK(c8->PC = START_ADDRESS);

    // cycle 3
    c8->KEYBOARD[0xC] = 1;
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->V[2] == 0xC);
    TEST_CHECK(c8->PC = START_ADDRESS + 2);

    chip8_destroy(&c8);
}

/**
 * DT = VX
 */
void test_0xFX15(void)
{
    uint8_t data[] = { 0xF4, 0x15 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);

    c8->V[4] = 60;
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->DT == 60);

    chip8_destroy(&c8);
}

/**
 * ST = VX
 */
void test_0xFX18(void)
{
    uint8_t data[] = { 0xF9, 0x18 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);

    c8->V[9] = 120;
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->ST == 120);

    chip8_destroy(&c8);
}

/**
 * I += VX
 */
void test_0xFX1E(void)
{
    uint8_t data[] = { 0xF1, 0x1E };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);

    c8->I = 0xC8;
    c8->V[1] = 42;
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->I == 0xC8 + 42);

    chip8_destroy(&c8);
}

/**
 * set I to the HEX char at VX
 */
void test_0xFX29(void)
{
    uint8_t data[] = { 0xF8, 0x29 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);
    c8->V[8] = 0xC;
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK_(c8->I == (FONTSET_ADDRESS + 0xC * FONT_OFFSET), "%d", c8->I);

    chip8_destroy(&c8);
}

/**
 * VX BCD
 */
void test_0xFX33(void)
{
    uint8_t data[] = { 0xF2, 0x33 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);

    c8->I = 0x300;
    c8->V[2] = 142;
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->RAM[0x300] == 1);
    TEST_CHECK(c8->RAM[0x301] == 4);
    TEST_CHECK(c8->RAM[0x302] == 2);

    chip8_destroy(&c8);
}

/**
 * store V0-VX
 */
void test_0xFX55(void)
{
    uint8_t data[] = { 0xF3, 0x55 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);

    for (int i = 0; i <= 3; i++) {
        c8->V[i] = i + 1;
    }
    c8->I = 0x400;
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    for (int i = 0; i <= 3; i++) {
        TEST_CHECK(c8->RAM[0x400 + i] == i + 1);
    }

    chip8_destroy(&c8);
}

/**
 * load V0-VX
 */
void test_0xFX65(void)
{
    uint8_t data[] = { 0xF3, 0x65, 1, 2, 3, 4 };
    instruction_t inst;
    exec_res_t result;
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 6);

    c8->I = START_ADDRESS + 2;
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    for (int i = 0; i <= 3; i++) {
        TEST_CHECK(c8->V[i] == i + 1);
    }

    chip8_destroy(&c8);
}

TEST_LIST = {
    { "PC overflow", test_PC_overflow },
    { "unknown opcode", test_unknown_opcode },
    { "0x0000 - no operation", test_0x0000 },
    { "0x00E0 - clear screen", test_0x00E0 },
    { "0x00EE - return from subroutine", test_0x00EE },
    { "0x00EE - stack underflow", test_0x00EE_stack_underflow },
    { "0x1NNN - jump to address NNN", test_0x1NNN },
    { "0x2NNN - call subroutine", test_0x2NNN },
    { "0x2NNN - stack overflow", test_0x2NNN_stack_overflow },
    { "0x3XNN - skip if VX == NN", test_0x3XNN },
    { "0x4XNN - skip if VX != NN", test_0x4XNN },
    { "0x5XY0 - skip if VX == VY", test_0x5XY0 },
    { "0x6XNN - VX = NN", test_0x6XNN },
    { "0x7XNN - VX += NN", test_0x7XNN },
    { "0x8XYA - unknown opcode", test_0x8XYA },
    { "0x8XY0 - VX = VY", test_0x8XY0 },
    { "0x8XY1 - VX |= VY", test_0x8XY1 },
    { "0x8XY2 - VX &= VY", test_0x8XY2 },
    { "0x8XY3 - VX ^= VY", test_0x8XY3 },
    { "0x8XY4 - VX += VY", test_0x8XY4 },
    { "0x8XY5 - VX -= VY", test_0x8XY5 },
    { "0x8XY6 - VX = VY >> 1", test_0x8XY6 },
    { "0x8XY7 - VX = VY - VX", test_0x8XY7 },
    { "0x8XYE - VX = VY << 1", test_0x8XYE },
    { "0x9XY0 - skip if VX != VY", test_0x9XY0 },
    { "0xANNN - I = NNN", test_0xANNN },
    { "0xBNNN - jump to address NNN + V0", test_0xBNNN },
    { "0xCXNN - VX = random NN", test_0xCXNN },
    { "0xDXYN - draw to (0, 0)", test_0xDXYN_00 },
    { "0xDXYN - draw to (X, Y) and erase", test_0xDXYN_XY_erase },
    { "0xDXYN - collision detection", test_0xDXYN_collision_detection },
    { "0xDXYN - draw with wrap and clip", test_0xDXYN_wrap_and_clip },
    { "0xEX9E - skip if VX key pressed", test_0xEX9E },
    { "0xEXA1 - skip if VX key not pressed", test_0xEXA1 },
    { "0xFX07 - VX = DT", test_0xFX07 },
    { "0xFX0A - wait, set VX = key", test_0xFX0A },
    { "0xFX15 - DT = VX", test_0xFX15 },
    { "0xFX18 - ST = VX", test_0xFX18 },
    { "0xFX1E - I += VX", test_0xFX1E },
    { "0xFX29 - set I to the HEX char at VX", test_0xFX29 },
    { "0xFX33 - VX BCD", test_0xFX33 },
    { "0xFX55 - store V0-VX", test_0xFX55 },
    { "0xFX65 - load V0-VX", test_0xFX65 },
    { NULL, NULL }
};

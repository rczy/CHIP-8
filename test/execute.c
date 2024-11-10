/**
 * Tests for instruction executing.
 */

#include "../lib/acutest.h"
#include "../src/chip8.c"

void create_chip8_snapshot(chip8_t *c8_src, chip8_t *c8_dest)
{
    c8_dest->I = c8_src->I;
    c8_dest->SP = c8_src->SP;
    c8_dest->PC = c8_src->PC;
    c8_dest->DT = c8_src->DT;
    c8_dest->ST = c8_src->ST;
    c8_dest->RF = c8_src->RF;
    memcpy(c8_dest->RAM, c8_src->RAM, sizeof(uint8_t) * RAM_SIZE);
    memcpy(c8_dest->V, c8_src->V, sizeof(uint8_t) * 16);
    memcpy(c8_dest->STACK, c8_src->STACK, sizeof(uint16_t) * STACK_SIZE);
    memcpy(c8_dest->SCREEN, c8_src->SCREEN, sizeof(uint8_t) * SCREEN_WIDTH * SCREEN_HEIGHT);
    memcpy(c8_dest->KEYBOARD, c8_src->KEYBOARD, sizeof(uint8_t) * 16);
}

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
    chip8_t *c8 = chip8_create(), c8_snapshot;
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);
    create_chip8_snapshot(c8, &c8_snapshot);
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(c8->I == c8_snapshot.I);
    TEST_CHECK(c8->SP == c8_snapshot.SP);
    TEST_CHECK(c8->PC == c8_snapshot.PC + 2);
    TEST_CHECK(c8->DT == c8_snapshot.DT);
    TEST_CHECK(c8->ST == c8_snapshot.ST);
    TEST_CHECK(c8->RF == c8_snapshot.RF);
    TEST_CHECK(memcmp(c8->RAM, c8_snapshot.RAM, sizeof(uint8_t) * RAM_SIZE) == 0);
    TEST_CHECK(memcmp(c8->V, c8_snapshot.V, sizeof(uint8_t) * 16) == 0);
    TEST_CHECK(memcmp(c8->STACK, c8_snapshot.STACK, sizeof(uint16_t) * STACK_SIZE) == 0);
    TEST_CHECK(memcmp(c8->SCREEN, c8_snapshot.SCREEN, sizeof(uint8_t) * SCREEN_WIDTH * SCREEN_HEIGHT) == 0);
    TEST_CHECK(memcmp(c8->KEYBOARD, c8_snapshot.KEYBOARD, sizeof(uint8_t) * 16) == 0);
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
    chip8_t *c8 = chip8_create(), c8_snapshot;
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);

    memset(c8->SCREEN, 1, sizeof(uint8_t) * SCREEN_WIDTH * SCREEN_HEIGHT);
    create_chip8_snapshot(c8, &c8_snapshot);

    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    // the following changed
    TEST_CHECK(c8->RF == 1);
    TEST_CHECK(memcmp(c8->SCREEN, c8_snapshot.SCREEN, sizeof(uint8_t) * SCREEN_WIDTH * SCREEN_HEIGHT) < 0);
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
    // the following changed
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
    // the following changed
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
    chip8_t *c8 = chip8_create(), c8_snapshot;
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);

    TEST_CHECK(c8->SP == 0);
    TEST_CHECK(c8->STACK[c8->SP] != START_ADDRESS);
    create_chip8_snapshot(c8, &c8_snapshot);

    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    // the following changed
    TEST_CHECK(c8->SP == c8_snapshot.SP + 1);
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
    TEST_CHECK(c8->V[0xA] == 0x04); // 14 - 10 = -4, underflow
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
    { NULL, NULL }
};

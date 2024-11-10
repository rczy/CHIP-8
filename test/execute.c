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
    // the following remained unchanged
    TEST_CHECK(c8->I == c8_snapshot.I);
    TEST_CHECK(c8->SP == c8_snapshot.SP);
    TEST_CHECK(c8->PC == c8_snapshot.PC + 2);
    TEST_CHECK(c8->DT == c8_snapshot.DT);
    TEST_CHECK(c8->ST == c8_snapshot.ST);
    TEST_CHECK(memcmp(c8->RAM, c8_snapshot.RAM, sizeof(uint8_t) * RAM_SIZE) == 0);
    TEST_CHECK(memcmp(c8->V, c8_snapshot.V, sizeof(uint8_t) * 16) == 0);
    TEST_CHECK(memcmp(c8->STACK, c8_snapshot.STACK, sizeof(uint16_t) * STACK_SIZE) == 0);
    TEST_CHECK(memcmp(c8->KEYBOARD, c8_snapshot.KEYBOARD, sizeof(uint8_t) * 16) == 0);
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
    chip8_t *c8 = chip8_create(), c8_snapshot;
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);

    TEST_CHECK(c8->SP == 0);
    TEST_CHECK(c8->STACK[c8->SP] != START_ADDRESS);
    c8->STACK[c8->SP] = START_ADDRESS;
    c8->SP++;
    create_chip8_snapshot(c8, &c8_snapshot);

    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    // the following remained unchanged
    TEST_CHECK(c8->I == c8_snapshot.I);
    TEST_CHECK(c8->DT == c8_snapshot.DT);
    TEST_CHECK(c8->ST == c8_snapshot.ST);
    TEST_CHECK(c8->RF == c8_snapshot.RF);
    TEST_CHECK(memcmp(c8->RAM, c8_snapshot.RAM, sizeof(uint8_t) * RAM_SIZE) == 0);
    TEST_CHECK(memcmp(c8->V, c8_snapshot.V, sizeof(uint8_t) * 16) == 0);
    TEST_CHECK(memcmp(c8->STACK, c8_snapshot.STACK, sizeof(uint16_t) * STACK_SIZE) == 0);
    TEST_CHECK(memcmp(c8->SCREEN, c8_snapshot.SCREEN, sizeof(uint8_t) * SCREEN_WIDTH * SCREEN_HEIGHT) == 0);
    TEST_CHECK(memcmp(c8->KEYBOARD, c8_snapshot.KEYBOARD, sizeof(uint8_t) * 16) == 0);
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
    chip8_t *c8 = chip8_create(), c8_snapshot;
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);

    c8->RAM[0x2C8] = 0xFF;
    c8->RAM[0x2C9] = 0xFF;

    create_chip8_snapshot(c8, &c8_snapshot);
    chip8_decode(chip8_fetch(c8), &inst);
    result = chip8_execute(c8, &inst);
    TEST_CHECK(result == EXEC_SUCCESS);
    TEST_CHECK(inst.NNN == 0x2C8);
    // the following remained unchanged
    TEST_CHECK(c8->I == c8_snapshot.I);
    TEST_CHECK(c8->SP == c8_snapshot.SP);
    TEST_CHECK(c8->DT == c8_snapshot.DT);
    TEST_CHECK(c8->ST == c8_snapshot.ST);
    TEST_CHECK(c8->RF == c8_snapshot.RF);
    TEST_CHECK(memcmp(c8->RAM, c8_snapshot.RAM, sizeof(uint8_t) * RAM_SIZE) == 0);
    TEST_CHECK(memcmp(c8->V, c8_snapshot.V, sizeof(uint8_t) * 16) == 0);
    TEST_CHECK(memcmp(c8->STACK, c8_snapshot.STACK, sizeof(uint16_t) * STACK_SIZE) == 0);
    TEST_CHECK(memcmp(c8->SCREEN, c8_snapshot.SCREEN, sizeof(uint8_t) * SCREEN_WIDTH * SCREEN_HEIGHT) == 0);
    TEST_CHECK(memcmp(c8->KEYBOARD, c8_snapshot.KEYBOARD, sizeof(uint8_t) * 16) == 0);
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
    // the following remained unchanged
    TEST_CHECK(c8->I == c8_snapshot.I);
    TEST_CHECK(c8->DT == c8_snapshot.DT);
    TEST_CHECK(c8->ST == c8_snapshot.ST);
    TEST_CHECK(c8->RF == c8_snapshot.RF);
    TEST_CHECK(memcmp(c8->RAM, c8_snapshot.RAM, sizeof(uint8_t) * RAM_SIZE) == 0);
    TEST_CHECK(memcmp(c8->V, c8_snapshot.V, sizeof(uint8_t) * 16) == 0);
    TEST_CHECK(memcmp(c8->SCREEN, c8_snapshot.SCREEN, sizeof(uint8_t) * SCREEN_WIDTH * SCREEN_HEIGHT) == 0);
    TEST_CHECK(memcmp(c8->KEYBOARD, c8_snapshot.KEYBOARD, sizeof(uint8_t) * 16) == 0);
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

TEST_LIST = {
    { "PC overflow", test_PC_overflow },
    { "unknown opcode", test_unknown_opcode },
    { "0x0000 - no operation", test_0x0000 },
    { "0x00E0 - clear screen", test_0x00E0 },
    { "0x00EE - return from subroutine", test_0x00EE },
    { "0x00EE - stack underflow", test_0x00EE_stack_underflow },
    { "0x1NNN - jump to address NNN", test_0x1NNN },
    { "0x2NNN - call subroutine", test_0x2NNN },
    { "0x2NNN - stack_overflow", test_0x2NNN_stack_overflow },
    { NULL, NULL }
};

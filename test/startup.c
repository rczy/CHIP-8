/**
 * Tests for CHIP-8 startup functions.
 */

#include "../lib/acutest.h"
#include "../src/chip8.c"

void test_create_destroy(void)
{
    chip8_t *c8 = chip8_create();
    TEST_CHECK_(c8 != NULL, "%p", c8);
    chip8_destroy(&c8);
    TEST_CHECK_(c8 == NULL, "%p", c8);
}

void test_reset(void)
{
    chip8_t *c8 = chip8_create();
    int sum = 0;
    TEST_CHECK(c8->I == 0);
    TEST_CHECK(c8->OP == 0);
    TEST_CHECK(c8->PC == 0);
    TEST_CHECK(c8->SP == 0);
    for (int i = 0; i < 16; i++) {
        sum += c8->V[i];
    }
    TEST_CHECK(sum == 0);
    TEST_CHECK(c8->DT == 0);
    TEST_CHECK(c8->ST == 0);
    sum = 0;
    for (int i = 0; i < STACK_SIZE; i++) {
        sum += c8->S[i];
    }
    TEST_CHECK(sum == 0);
    sum = 0;
    for (int i = 0; i < RAM_SIZE; i++) {
        sum += c8->RAM[i];
    }
    TEST_CHECK(sum == 0);
    chip8_destroy(&c8);
}

void test_ramcpy(void)
{
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, "CHIP-8", 7);
    TEST_CHECK(memcmp(c8->RAM, "CHIP-8", 7) != 0);
    TEST_CHECK(memcmp(&c8->RAM[START_ADDRESS], "CHIP-8", 7) == 0);
    chip8_ramcpy(c8, "W", 1);
    TEST_CHECK(memcmp(&c8->RAM[START_ADDRESS], "CHIP-8", 7) != 0);
    TEST_CHECK(memcmp(&c8->RAM[START_ADDRESS], "WHIP-8", 7) == 0);
    chip8_destroy(&c8);
}

TEST_LIST = {
    { "create and destroy",  test_create_destroy},
    { "reset",  test_reset},
    { "copy bytes to RAM",  test_ramcpy},
    { NULL, NULL }
};
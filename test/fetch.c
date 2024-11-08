/**
 * Tests for opcode fetching.
 */

#include "../lib/acutest.h"
#include "../src/chip8.c"

void test_fetch_initial(void)
{
    uint8_t data[] = { 0xC8, 0x12 };
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);
    TEST_CHECK(c8->RAM[START_ADDRESS] == 0xC8);
    TEST_CHECK(c8->RAM[START_ADDRESS + 1] == 0x12);
    uint16_t opcode = chip8_fetch(c8);
    TEST_CHECK(opcode == 0xC812);
    TEST_CHECK(c8->PC == START_ADDRESS + 2);
    chip8_destroy(&c8);
}

void test_fetch_twice(void)
{
    uint8_t data[] = { 0xC8, 0x12, 0x34, 0x56 };
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 4);
    uint16_t opcode = chip8_fetch(c8);
    TEST_CHECK(opcode == 0xC812);
    opcode = chip8_fetch(c8);
    TEST_CHECK(opcode == 0x3456);
    TEST_CHECK(c8->PC == START_ADDRESS + 4);
    chip8_destroy(&c8);
}

void test_fetch_skip_one(void)
{
    uint8_t data[] = { 0xC8, 0x12, 0x34, 0x56, 0x78, 0x9A };
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 6);
    uint16_t opcode = chip8_fetch(c8);
    TEST_CHECK(opcode == 0xC812);
    c8->PC += 2;
    opcode = chip8_fetch(c8);
    TEST_CHECK(opcode == 0x789A);
    TEST_CHECK(c8->PC == START_ADDRESS + 6);
    chip8_destroy(&c8);
}

void test_fetch_arbitary(void)
{
    uint8_t data[] = { 0xC8, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE };
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 8);
    c8->PC += 6;
    uint16_t opcode = chip8_fetch(c8);
    TEST_CHECK(opcode == 0xBCDE);
    TEST_CHECK(c8->PC == START_ADDRESS + 6);
    chip8_destroy(&c8);
}

TEST_LIST = {
    { "initial opcode fetch", test_fetch_initial },
    { "second opcode fetch", test_fetch_twice },
    { "skip second opcode", test_fetch_skip_one },
    { "arbitary opcode fetch", test_fetch_skip_one },
    { NULL, NULL }
};

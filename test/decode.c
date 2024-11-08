/**
 * Tests for opcode decoding.
 */

#include "../lib/acutest.h"
#include "../src/chip8.c"

void test_decode_opcode(void)
{
    uint8_t data[] = { 0xC8, 0x12 };
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);
    chip8_ramcpy(c8, data, 2);
    uint16_t opcode = chip8_fetch(c8);
    instruction_t inst;
    chip8_decode(opcode, &inst);
    TEST_CHECK(inst.OP == 0xC812);
    TEST_CHECK(inst.X == 0x8);
    TEST_CHECK(inst.Y == 1);
    TEST_CHECK(inst.N == 2);
    TEST_CHECK(inst.NN == 0x12);
    TEST_CHECK(inst.NNN == 0x812);
    chip8_destroy(&c8);
}

TEST_LIST = {
    { "decode opcode", test_decode_opcode },
    { NULL, NULL }
};

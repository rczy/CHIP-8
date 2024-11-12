/**
 * Tests for CHIP-8 setup functions.
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
    chip8_reset(c8);
    TEST_CHECK(c8->I == 0);
    TEST_CHECK(c8->PC == START_ADDRESS);
    TEST_CHECK(c8->SP == 0);
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
    for (int i = 0; i < RAM_SIZE; i++) {
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

void test_load_rom(void)
{
    chip8_t *c8 = chip8_create();
    chip8_reset(c8);

    FILE *no_rom = NULL;
    TEST_CHECK(chip8_load_rom(c8, no_rom) == ROM_NOT_EXISTS);

    FILE *large_rom = fopen("bin/test/large_rom.ch8", "wb+");
    uint8_t large_data[RAM_SIZE] = { 0 };
    fwrite(large_data, sizeof(uint8_t), RAM_SIZE, large_rom);
    TEST_CHECK(chip8_load_rom(c8, large_rom) == ROM_TOO_LARGE);
    fclose(large_rom);

    FILE *rom = fopen("bin/test/rom.ch8", "wb+");
    uint8_t data[] = { 0x43, 0x48, 0x49, 0x50, 0x2D, 0x38 };
    fwrite(data, sizeof(uint8_t), 6, rom);
    TEST_CHECK(chip8_load_rom(c8, rom) == ROM_LOAD_SUCCESS);
    fclose(rom);
    TEST_CHECK(memcmp(&c8->RAM[START_ADDRESS], data, 6) == 0);
}

TEST_LIST = {
    { "create and destroy",  test_create_destroy},
    { "reset",  test_reset},
    { "copy bytes to RAM",  test_ramcpy},
    { "load ROM file into RAM",  test_load_rom},
    { NULL, NULL }
};

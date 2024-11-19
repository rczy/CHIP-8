#ifndef ARGS_H
#define ARGS_H

#include <stdint.h>

#define IPF 9
#define TONE 440
#define BG_COLOR 0x000000
#define FG_COLOR 0x00FF00

typedef struct args_t {
    char *rom_path;
    uint8_t ipf;
    uint16_t tone;
    uint32_t bg_color;
    uint32_t fg_color;
} args_t;

args_t parse_args(int argc, char *argv[]);

#endif

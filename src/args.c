#include <stdlib.h>
#include <string.h>
#include "include/args.h"

args_t parse_args(int argc, char *argv[])
{
    args_t args = {
        .ipf = IPF,
        .tone = TONE,
        .bg_color = BG_COLOR,
        .fg_color = FG_COLOR
    };
    if (argc > 1) {
        args.rom_path = argv[1];
    }
    for (int i = 2; i + 1 < argc; i += 2) {
        if (strcmp("--ipf", argv[i]) == 0) {
            args.ipf = strtol(argv[i + 1], NULL, 10);
        } else if (strcmp("--tone", argv[i]) == 0) {
            args.tone = strtol(argv[i + 1], NULL, 10);
        } else if (strcmp("--bg-color", argv[i]) == 0) {
            args.bg_color = strtol(argv[i + 1], NULL, 16);
        } else if (strcmp("--fg-color", argv[i]) == 0) {
            args.fg_color = strtol(argv[i + 1], NULL, 16);
        }
    }
    return args;
}
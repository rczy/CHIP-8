#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "include/args.h"
#include "include/device.h"

device_t *device;

void clean_up();

int main(int argc, char *argv[])
{
    if (argc < 2 || SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        return EXIT_FAILURE;
    }

    args_t args = parse_args(argc, argv);
    device = device_init(&args);

    atexit(clean_up);

    if (device_start(device) != ROM_LOAD_SUCCESS) {
        return EXIT_FAILURE;
    }

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(device_iterate, device, 60, 0);
#else
    while (device->running) {
        device_iterate(device);
    }
#endif

    return EXIT_SUCCESS;
}

void clean_up()
{
    device_destroy(&device);
    SDL_Quit();
}

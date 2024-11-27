#include <stdlib.h>
#include <SDL2/SDL.h>
#include "include/args.h"
#include "include/device.h"

int main(int argc, char *argv[])
{
    if (argc < 2 || SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        return EXIT_FAILURE;
    }

    args_t args = parse_args(argc, argv);
    device_t *device = device_init(&args);
    
    if (device_start(device) != ROM_LOAD_SUCCESS) {
        return EXIT_FAILURE;
    }

    while (device->running) {
        device_iterate(device);
    }

    device_destroy(&device);
    SDL_Quit();

    return EXIT_SUCCESS;
}

#include <SDL2/SDL.h>
#include "include/device.h"

int main(int argc, char *argv[])
{
    if (argc < 2 || SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        return EXIT_FAILURE;
    }

    uint16_t ipf = argc > 2 ? strtol(argv[2], NULL, 10) : DEFAULT_IPF;
    device_t *device = device_init(argv[1], ipf);
    
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

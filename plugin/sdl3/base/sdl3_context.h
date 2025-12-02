#pragma once

#include <SDL3/SDL.h>

namespace ark::plugin::sdl3 {

struct SDL3_Context {
    SDL_Window* _main_window;
};

}

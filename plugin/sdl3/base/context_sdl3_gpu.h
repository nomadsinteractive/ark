#pragma once

#include <SDL3/SDL.h>

#include "renderer/forwarding.h"

namespace ark::plugin::sdl3 {

struct ContextSDL3_GPU {
    ContextSDL3_GPU();
    ~ContextSDL3_GPU();

    SDL_Window* _main_window;
    SDL_GPUDevice* _gpu_gevice;
};

struct GraphicsContextSDL3_GPU {
    SDL_GPUColorTargetInfo _color_target_info;
    SDL_GPUCommandBuffer* _command_buffer;
};

const ContextSDL3_GPU& ensureContext(GraphicsContext& graphicsContext);
SDL_GPUDevice* ensureGPUDevice(GraphicsContext& graphicsContext);

}
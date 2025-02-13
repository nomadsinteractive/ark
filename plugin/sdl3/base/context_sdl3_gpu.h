#pragma once

#include <SDL3/SDL.h>

#include "renderer/forwarding.h"

namespace ark::plugin::sdl3 {

struct ContextSDL3_GPU {
    SDL_Window* _main_window;
    SDL_GPUDevice* _gpu_gevice;
    SDL_GPUTexture* _swapchain_texture;
};

struct GraphicsContextSDL3_GPU {
    SDL_GPUTexture* _render_target;
    SDL_GPUCommandBuffer* _command_buffer;
};

SDL_GPUDevice* ensureGPUDevice(GraphicsContext& graphicsContext);

}
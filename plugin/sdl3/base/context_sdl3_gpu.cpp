#include "sdl3/base/context_sdl3_gpu.h"

#include <SDL3_shadercross/SDL_shadercross.h>

#include "core/collection/traits.h"

#include "renderer/base/graphics_context.h"

namespace ark::plugin::sdl3 {

ContextSDL3_GPU::ContextSDL3_GPU()
{
    SDL_ShaderCross_Init();
}

ContextSDL3_GPU::~ContextSDL3_GPU()
{
    SDL_ShaderCross_Quit();
}

const ContextSDL3_GPU& ensureContext(GraphicsContext& graphicsContext)
{
    const sp<ContextSDL3_GPU> context = graphicsContext.traits().ensure<ContextSDL3_GPU>();
    ASSERT(context);
    return *context;
}

SDL_GPUDevice* ensureGPUDevice(GraphicsContext& graphicsContext)
{
    SDL_GPUDevice* gpuDevice = graphicsContext.traits().ensure<ContextSDL3_GPU>()->_gpu_gevice;
    ASSERT(gpuDevice);
    return gpuDevice;
}

}


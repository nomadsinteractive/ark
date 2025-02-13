#include "sdl3/base/context_sdl3_gpu.h"

#include "core/collection/traits.h"

#include "renderer/base/graphics_context.h"

namespace ark::plugin::sdl3 {

SDL_GPUDevice* ensureGPUDevice(GraphicsContext& graphicsContext)
{
    SDL_GPUDevice* gpuDevice = graphicsContext.traits().ensure<ContextSDL3_GPU>()->_gpu_gevice;
    ASSERT(gpuDevice);
    return gpuDevice;
}

}


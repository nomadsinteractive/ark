#include "context_sdl3_gpu.h"

#include <SDL3_shadercross/SDL_shadercross.h>

#include "core/collection/traits.h"

#include "renderer/base/graphics_context.h"

namespace ark::plugin::sdl3 {

SDL3_GPU_Context::SDL3_GPU_Context()
{
    SDL_ShaderCross_Init();
}

SDL3_GPU_Context::~SDL3_GPU_Context()
{
    SDL_ShaderCross_Quit();
}

void GraphicsContextSDL3_GPU::pushRenderTargets(const RenderTarget::Configure* createConfig, const Vector<SDL_GPUColorTargetInfo>& colorTargets, const Optional<SDL_GPUDepthStencilTargetInfo>& depthStencilTarget)
{
    ASSERT(!depthStencilTarget || depthStencilTarget->texture);
    _render_targets.push_back({createConfig, colorTargets, depthStencilTarget});
}

void GraphicsContextSDL3_GPU::popRenderTargets()
{
    _render_targets.pop_back();
}

const RenderTargetContext& GraphicsContextSDL3_GPU::renderTarget() const
{
    return _render_targets.back();
}

const SDL3_GPU_Context& ensureGPUContext(GraphicsContext& graphicsContext)
{
    const sp<SDL3_GPU_Context> context = graphicsContext.traits().ensure<SDL3_GPU_Context>();
    ASSERT(context);
    return *context;
}

GraphicsContextSDL3_GPU& ensureGraphicsContext(GraphicsContext& graphicsContext)
{
    const sp<GraphicsContextSDL3_GPU> context = graphicsContext.traits().ensure<GraphicsContextSDL3_GPU>();
    ASSERT(context);
    return *context;
}

SDL_GPUDevice* ensureGPUDevice(GraphicsContext& graphicsContext)
{
    SDL_GPUDevice* gpuDevice = graphicsContext.traits().ensure<SDL3_GPU_Context>()->_gpu_gevice;
    ASSERT(gpuDevice);
    return gpuDevice;
}

}


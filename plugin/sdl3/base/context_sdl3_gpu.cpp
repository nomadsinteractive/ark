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

void SDL3_GPU_GraphicsContext::pushRenderTargets(const RenderTarget::Configure* createConfig, const Vector<SDL_GPUColorTargetInfo>& colorTargets, const Optional<SDL_GPUDepthStencilTargetInfo>& depthStencilTarget)
{
    ASSERT(!depthStencilTarget || depthStencilTarget->texture);
    ASSERT(_active_render_target_index < static_cast<int32_t>(array_size(_render_targets) - 1));
    _render_targets[++_active_render_target_index] = {createConfig, &colorTargets, &depthStencilTarget};
}

void SDL3_GPU_GraphicsContext::popRenderTargets()
{
    ASSERT(_active_render_target_index >= 0);
    _active_render_target_index --;
}

const RenderTargetContext& SDL3_GPU_GraphicsContext::getCurrentRenderTarget()
{
    ASSERT(_active_render_target_index >= 0);
    const RenderTargetContext& rt = _render_targets[_active_render_target_index];
    if(_active_render_target_index == 0)
        _render_targets[++_active_render_target_index] = _rt_swapchain_blend;
    return rt;
}

const SDL3_GPU_Context& ensureGPUContext(GraphicsContext& graphicsContext)
{
    const sp<SDL3_GPU_Context> context = graphicsContext.traits().ensure<SDL3_GPU_Context>();
    ASSERT(context);
    return *context;
}

SDL3_GPU_GraphicsContext& ensureGraphicsContext(GraphicsContext& graphicsContext)
{
    const sp<SDL3_GPU_GraphicsContext> context = graphicsContext.traits().ensure<SDL3_GPU_GraphicsContext>();
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


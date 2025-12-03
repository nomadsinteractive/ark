#pragma once

#include <SDL3/SDL.h>

#include "renderer/forwarding.h"
#include "renderer/base/render_target.h"

namespace ark::plugin::sdl3 {

struct SDL3_GPU_Context {
    SDL3_GPU_Context();
    ~SDL3_GPU_Context();

    SDL_GPUDevice* _gpu_gevice;
};

struct RenderTargetContext {
    const RenderTarget::Configure* _create_config;
    const Vector<SDL_GPUColorTargetInfo>* _color_targets;
    const Optional<SDL_GPUDepthStencilTargetInfo>* _depth_stencil_target;
};

struct SDL3_GPU_GraphicsContext {
    SDL_GPUCommandBuffer* _command_buffer;

    RenderTargetContext _rt_swapchain_blend;

    RenderTargetContext _render_targets[32] = {};
    int32_t _active_render_target_index = -1;

    const RenderTargetContext& getCurrentRenderTarget();

    void pushRenderTargets(const RenderTarget::Configure* createConfig, const Vector<SDL_GPUColorTargetInfo>& colorTargets, const Optional<SDL_GPUDepthStencilTargetInfo>& depthStencilTarget);
    void popRenderTargets();
};

const SDL3_GPU_Context& ensureGPUContext(GraphicsContext& graphicsContext);
SDL3_GPU_GraphicsContext& ensureGraphicsContext(GraphicsContext& graphicsContext);

SDL_GPUDevice* ensureGPUDevice(GraphicsContext& graphicsContext);

}

#pragma once

#include <SDL3/SDL.h>

#include "core/types/optional.h"

#include "renderer/forwarding.h"
#include "renderer/base/render_target.h"

namespace ark::plugin::sdl3 {

struct ContextSDL3_GPU {
    ContextSDL3_GPU();
    ~ContextSDL3_GPU();

    SDL_Window* _main_window;
    SDL_GPUDevice* _gpu_gevice;
};

struct RenderTargetContext {
    const RenderTarget::Configure* _create_config;
    const Vector<SDL_GPUColorTargetInfo>& _color_targets;
    const Optional<SDL_GPUDepthStencilTargetInfo>& _depth_stencil_target;
};

struct GraphicsContextSDL3_GPU {
    SDL_GPUCommandBuffer* _command_buffer;

    Vector<RenderTargetContext> _render_targets;

    const RenderTargetContext& renderTarget() const;
    void pushRenderTargets(const RenderTarget::Configure* createConfig, const Vector<SDL_GPUColorTargetInfo>& colorTargets, const Optional<SDL_GPUDepthStencilTargetInfo>& depthStencilTarget);
    void popRenderTargets();
};

const ContextSDL3_GPU& ensureContext(GraphicsContext& graphicsContext);
GraphicsContextSDL3_GPU& ensureGraphicsContext(GraphicsContext& graphicsContext);

SDL_GPUDevice* ensureGPUDevice(GraphicsContext& graphicsContext);

}

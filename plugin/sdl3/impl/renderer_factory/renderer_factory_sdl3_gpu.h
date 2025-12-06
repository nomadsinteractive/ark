#pragma once

#include <SDL3/SDL.h>

#include "renderer/inf/renderer_factory.h"

namespace ark::plugin::sdl3 {

class RendererFactorySDL3_GPU final : public RendererFactory {
public:
    RendererFactorySDL3_GPU();

    void onSurfaceCreated(RenderEngine& renderEngine) override;

    sp<RenderEngineContext> createRenderEngineContext(const ApplicationManifest::Renderer& renderer) override;
    sp<Buffer::Delegate> createBuffer(Buffer::Usage usage) override;
    sp<RenderTarget> createRenderTarget(sp<Renderer> renderer, RenderTarget::Configure configure) override;
    sp<PipelineFactory> createPipelineFactory() override;
    sp<RenderView> createRenderView(const sp<RenderEngineContext>& renderContext, const sp<RenderController>& renderController) override;
    sp<Texture::Delegate> createTexture(sp<Size> size, sp<Texture::Parameters> parameters) override;

private:
    SDL_GPUDevice* _gpu_device;
};

}

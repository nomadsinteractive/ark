#pragma once

#include <SDL3/SDL.h>

#include "renderer/inf/renderer_factory.h"

namespace ark::plugin::sdl3 {

class RendererFactorySDL3_GPU final : public RendererFactory {
public:
    RendererFactorySDL3_GPU();

    void onSurfaceCreated(RenderEngine& renderEngine) override;

    sp<RenderEngineContext> createRenderEngineContext(const ApplicationManifest::Renderer& renderer) override;
    sp<Buffer::Delegate> createBuffer(Buffer::Type type, Buffer::Usage usage) override;
    sp<Camera::Delegate> createCamera(Ark::RendererCoordinateSystem rcs) override;
    sp<RenderTarget> createRenderTarget(sp<Renderer> renderer, RenderTarget::CreateConfigure configure) override;
    sp<PipelineFactory> createPipelineFactory() override;
    sp<RenderView> createRenderView(const sp<RenderEngineContext>& renderContext, const sp<RenderController>& renderController) override;
    sp<Texture::Delegate> createTexture(sp<Size> size, sp<Texture::Parameters> parameters) override;

//  [[plugin::builder::by-value("sdl3-gpu")]]
    class BUILDER final : public Builder<RendererFactory> {
    public:
        BUILDER() = default;

        sp<RendererFactory> build(const Scope& args) override;
    };

private:
    SDL_GPUDevice* _gpu_device;
};

}

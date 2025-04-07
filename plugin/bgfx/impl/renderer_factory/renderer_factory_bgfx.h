#pragma once

#include "renderer/inf/renderer_factory.h"

namespace ark::plugin::bgfx {

class RendererFactoryBgfx final : public RendererFactory {
public:
    RendererFactoryBgfx();

    void onSurfaceCreated(RenderEngine& renderEngine) override;

    sp<RenderEngineContext> createRenderEngineContext(const ApplicationManifest::Renderer& renderer) override;
    sp<Buffer::Delegate> createBuffer(Buffer::Usage usage) override;
    sp<Camera::Delegate> createCamera(Ark::RendererCoordinateSystem rcs) override;
    sp<RenderTarget> createRenderTarget(sp<Renderer> renderer, RenderTarget::Configure configure) override;
    sp<PipelineFactory> createPipelineFactory() override;
    sp<RenderView> createRenderView(const sp<RenderEngineContext>& renderContext, const sp<RenderController>& renderController) override;
    sp<Texture::Delegate> createTexture(sp<Size> size, sp<Texture::Parameters> parameters) override;

//  [[plugin::builder::by-value("bgfx")]]
    class BUILDER final : public Builder<RendererFactory> {
    public:
        BUILDER() = default;

        sp<RendererFactory> build(const Scope& args) override;
    };

};

}

#pragma once

#include "core/ark.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "renderer/inf/renderer_factory.h"
#include "renderer/forwarding.h"

namespace ark::plugin::opengl {

class RendererFactoryOpenGL final : public RendererFactory {
public:
    RendererFactoryOpenGL();

    void onSurfaceCreated(RenderEngine& renderEngine) override;

    sp<RenderEngineContext> createRenderEngineContext(const ApplicationManifest::Renderer& renderer) override;
    sp<Buffer::Delegate> createBuffer(Buffer::Type type, Buffer::Usage usage) override;
    sp<Camera::Delegate> createCamera(Ark::RendererCoordinateSystem rcs) override;
    sp<RenderTarget> createRenderTarget(sp<RenderLayer> renderLayer, RenderTarget::Configure configure) override;
    sp<RenderView> createRenderView(const sp<RenderEngineContext>& renderContext, const sp<RenderController>& renderController) override;
    sp<PipelineFactory> createPipelineFactory() override;
    sp<Texture::Delegate> createTexture(sp<Size> size, sp<Texture::Parameters> parameters) override;

//  [[plugin::builder::by-value("opengl")]]
    class BUILDER final : public Builder<RendererFactory> {
    public:
        BUILDER() = default;

        sp<RendererFactory> build(const Scope& args) override;
    };
};

}

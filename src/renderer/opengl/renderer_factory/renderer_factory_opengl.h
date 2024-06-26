#pragma once

#include "core/ark.h"
#include "core/types/shared_ptr.h"

#include "renderer/inf/renderer_factory.h"
#include "renderer/forwarding.h"

namespace ark::opengl {

class RendererFactoryOpenGL : public RendererFactory {
public:
    RendererFactoryOpenGL(const sp<Recycler>& recycler);

    sp<RenderEngineContext> initialize(Ark::RendererVersion version) override;

    void onSurfaceCreated(RenderEngineContext& glContext) override;

    sp<Buffer::Delegate> createBuffer(Buffer::Type type, Buffer::Usage usage) override;
    sp<Camera::Delegate> createCamera(Ark::RendererCoordinateSystem cs) override;
    sp<Framebuffer> createFramebuffer(sp<Renderer> renderer, std::vector<sp<Texture>> colorAttachments, sp<Texture> depthStencilAttachments, int32_t clearMask) override;
    sp<RenderView> createRenderView(const sp<RenderEngineContext>& renderContext, const sp<RenderController>& renderController) override;
    sp<PipelineFactory> createPipelineFactory() override;
    sp<Texture::Delegate> createTexture(sp<Size> size, sp<Texture::Parameters> parameters) override;

private:
    void setVersion(Ark::RendererVersion version, RenderEngineContext& glContext);

private:
    sp<Recycler> _recycler;

};

}

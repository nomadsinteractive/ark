#pragma once

#include "renderer/inf/renderer_factory.h"

namespace ark::plugin::bgfx {

class RendererFactoryBgfx : public RendererFactory {
public:

    void onSurfaceCreated(RenderEngine& renderEngine) override;

    sp<RenderEngineContext> createRenderEngineContext(Ark::RendererVersion version) override;
    sp<Buffer::Delegate> createBuffer(Buffer::Type type, Buffer::Usage usage) override;
    sp<Camera::Delegate> createCamera() override;
    sp<Framebuffer> createFramebuffer(sp<Renderer> renderer, std::vector<sp<Texture>> colorAttachments, sp<Texture> depthStencilAttachments, int32_t clearMask) override;
    sp<PipelineFactory> createPipelineFactory() override;
    sp<RenderView> createRenderView(const sp<RenderEngineContext>& renderContext, const sp<RenderController>& renderController) override;
    sp<Texture::Delegate> createTexture(sp<Size> size, sp<Texture::Parameters> parameters) override;

};

}

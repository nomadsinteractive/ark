#ifndef ARK_RENDERER_VULKAN_RENDERER_FACTORY_RENDERER_FACTORY_VULKAN_H_
#define ARK_RENDERER_VULKAN_RENDERER_FACTORY_RENDERER_FACTORY_VULKAN_H_

#include "core/ark.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/renderer_factory.h"

#include "renderer/vulkan/forward.h"

namespace ark {
namespace vulkan {

class RendererFactoryVulkan : public RendererFactory {
public:
    RendererFactoryVulkan(const sp<Recycler>& recycler);
    ~RendererFactoryVulkan() override;

    virtual sp<RenderEngineContext> initialize(Ark::RendererVersion version) override;
    virtual void onSurfaceCreated(RenderEngineContext& glContext) override;

    virtual sp<Buffer::Delegate> createBuffer(Buffer::Type type, Buffer::Usage usage) override;
    virtual sp<Camera::Delegate> createCamera(Ark::RendererCoordinateSystem cs) override;
    virtual sp<Resource> createFramebuffer(const std::vector<sp<Texture>>& colorAttachments, const sp<Texture>& depthStencilAttachments) override;
    virtual sp<Renderer> createFramebufferRenderer(RenderController& renderController, sp<Framebuffer> framebuffer, sp<Renderer> delegate, std::vector<sp<Texture>> drawBuffers, int32_t clearMask) override;
    virtual sp<RenderView> createRenderView(const sp<RenderEngineContext>& renderContext, const sp<RenderController>& renderController) override;
    virtual sp<PipelineFactory> createPipelineFactory() override;
    virtual sp<Texture::Delegate> createTexture(const sp<Size>& size, const sp<Texture::Parameters>& parameters, const sp<Texture::Uploader>& uploader) override;

private:
    void setVersion(Ark::RendererVersion version, RenderEngineContext& vkContext);

private:
    sp<Recycler> _recycler;
    sp<VKRenderer> _renderer;

    friend class VKUtil;
    friend class PipelineFactoryVulkan;

};

}
}

#endif

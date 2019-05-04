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

    virtual sp<RenderContext> initialize(Ark::RendererVersion version) override;
    virtual void onSurfaceCreated(RenderContext& glContext) override;

    virtual sp<Buffer::Delegate> createBuffer(Buffer::Type type, Buffer::Usage usage) override;
    virtual sp<Framebuffer> createFramebuffer(const sp<Renderer>& renderer, const sp<Texture>& texture) override;
    virtual sp<RenderView> createRenderView(const sp<RenderContext>& renderContext, const sp<RenderController>& renderController) override;
    virtual sp<PipelineFactory> createPipelineFactory() override;
    virtual sp<Texture> createTexture(uint32_t width, uint32_t height, const sp<Texture::Uploader>& uploader) override;

private:
    void setVersion(Ark::RendererVersion version, RenderContext& vkContext);

private:
    sp<Recycler> _recycler;
    sp<VKRenderer> _renderer;

    friend class VKUtil;
    friend class PipelineFactoryVulkan;

};

}
}

#endif

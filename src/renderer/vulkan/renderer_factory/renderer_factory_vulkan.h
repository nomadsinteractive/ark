#ifndef ARK_RENDERER_VULKAN_RENDERER_FACTORY_RENDERER_FACTORY_VULKAN_H_
#define ARK_RENDERER_VULKAN_RENDERER_FACTORY_RENDERER_FACTORY_VULKAN_H_

#include "core/ark.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/renderer_factory.h"
#include "renderer/vulkan/base/vulkan_api.h"

namespace ark {
namespace vulkan {

class RendererFactoryVulkan : public RendererFactory {
public:
    RendererFactoryVulkan(const sp<GLResourceManager>& glResources);
    ~RendererFactoryVulkan() override;

    virtual void initialize(GLContext& glContext) override;

    virtual void setGLVersion(Ark::GLVersion version, GLContext& glContext) override;

    virtual sp<RenderView> createRenderView(const sp<GLContext>& glContext, const Viewport& viewport) override;

private:
    sp<GLResourceManager> _resource_manager;

    sp<VulkanAPI> _vulkan_api;

};

}
}
#endif

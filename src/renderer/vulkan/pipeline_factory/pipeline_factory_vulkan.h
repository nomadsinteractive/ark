#ifndef ARK_RENDERER_VULKAN_PIPELINE_FACTORY_PIPELINE_FACTORY_VULKAN_H_
#define ARK_RENDERER_VULKAN_PIPELINE_FACTORY_PIPELINE_FACTORY_VULKAN_H_

#include <vector>

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/pipeline_factory.h"
#include "renderer/vulkan/forward.h"

namespace ark {
namespace vulkan {

class PipelineFactoryVulkan : public PipelineFactory {
public:
    PipelineFactoryVulkan(const sp<ResourceManager>& resourceManager, const sp<VKRenderer>& renderFactory);

    virtual sp<Pipeline> buildPipeline(GraphicsContext& graphicsContext, const sp<ShaderBindings>& shaderBindings) override;

private:
    sp<ResourceManager> _resource_manager;
    sp<VKRenderer> _renderer;
};

}
}

#endif

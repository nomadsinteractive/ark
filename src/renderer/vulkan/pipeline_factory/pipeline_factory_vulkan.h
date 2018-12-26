#ifndef ARK_RENDERER_VULKAN_PIPELINE_FACTORY_PIPELINE_FACTORY_VULKAN_H_
#define ARK_RENDERER_VULKAN_PIPELINE_FACTORY_PIPELINE_FACTORY_VULKAN_H_

#include <vector>

#include <vulkan/vulkan.h>

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/pipeline_factory.h"
#include "renderer/vulkan/renderer_factory/renderer_factory_vulkan.h"
#include "renderer/vulkan/forward.h"

namespace ark {
namespace vulkan {

class PipelineFactoryVulkan : public PipelineFactory {
public:
    PipelineFactoryVulkan(const sp<ResourceManager>& resourceManager, const sp<VKRenderer>& renderFactory);
    ~PipelineFactoryVulkan() override;

    sp<VKPipeline> build();

    virtual sp<Pipeline> buildPipeline(GraphicsContext& graphicsContext, const PipelineLayout& pipelineLayout, const ShaderBindings& bindings) override;

    sp<VKBuffer> _ubo;
    sp<VKTexture2D> _texture;

private:
    void setupVertexDescriptions(const PipelineInput& input);
    void setupVertexDescriptions();
    void setupDescriptorSetLayout();
    void setupDescriptorPool();

    VkDescriptorSet setupDescriptorSet();
    VkDescriptorSet setupDescriptorSet(const bytearray& ubo, const ShaderBindings& bindings);
    VkPipeline createPipeline();

    VkFormat getFormat(const Attribute& attribute) const;

private:
    sp<ResourceManager> _resource_manager;
    sp<VKRenderer> _renderer;

    VkPipelineLayout _pipeline_layout;
    VkDescriptorSetLayout _descriptor_set_layout;

    struct {
        VkPipelineVertexInputStateCreateInfo inputState;
        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    } vertices;

    std::unordered_map<String, uint32_t> _location_map;
};

}
}

#endif

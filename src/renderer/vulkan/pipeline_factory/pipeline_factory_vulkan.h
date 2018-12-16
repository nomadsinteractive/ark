#ifndef ARK_RENDERER_VULKAN_PIPELINE_FACTORY_PIPELINE_FACTORY_VULKAN_H_
#define ARK_RENDERER_VULKAN_PIPELINE_FACTORY_PIPELINE_FACTORY_VULKAN_H_

#include <vector>

#include <vulkan/vulkan.h>

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/pipeline_factory.h"
#include "renderer/vulkan/forward.h"

namespace ark {
namespace vulkan {

class PipelineFactoryVulkan : public PipelineFactory {
public:
    PipelineFactoryVulkan(const sp<ResourceManager>& resourceManager, const sp<VKRenderTarget>& renderTarget);
    ~PipelineFactoryVulkan() override;

    sp<VKPipeline> build();

    virtual sp<Pipeline> buildPipeline(GraphicsContext& graphicsContext, const PipelineLayout& _pipeline_layout) override;
    virtual sp<RenderCommand> buildRenderCommand(ObjectPool& objectPool, DrawingContext drawingContext, const sp<Shader>& shader, RenderModel::Mode renderMode, int32_t count) override;

    sp<VKBuffer> _ubo;
    sp<VKTexture2D> _texture;

private:
    void setupVertexDescriptions(const PipelineInput& input);
    void setupVertexDescriptions();
    void setupDescriptorSetLayout();
    void setupDescriptorPool();

    VkDescriptorSet setupDescriptorSet();
    VkPipeline preparePipelines();

    VkFormat getFormat(const Attribute& attribute) const;

private:
    sp<ResourceManager> _resource_manager;
    sp<VKRenderTarget> _render_target;
    sp<VKDevice> _device;

    VkPipelineLayout _pipeline_layout;
    VkDescriptorSetLayout _descriptor_set_layout;

    VkDescriptorPool _descriptor_pool;

    struct {
        VkPipelineVertexInputStateCreateInfo inputState;
        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    } vertices;

    std::vector<VkCommandBuffer> _command_buffers;
    std::unordered_map<String, uint32_t> _location_map;
};

}
}

#endif

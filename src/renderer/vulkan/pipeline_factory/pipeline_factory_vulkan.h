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
    PipelineFactoryVulkan(const sp<GLResourceManager>& resourceManager);
    ~PipelineFactoryVulkan() override;

    sp<Pipeline> build(const sp<RenderTarget>& renderTarget);

    virtual sp<ark::Pipeline> buildPipeline(GraphicsContext& graphicsContext, const PipelineLayout& pipelineLayout) override;
    virtual sp<RenderCommand> buildRenderCommand(ObjectPool& objectPool, DrawingContext drawingContext, const sp<Shader>& shader, RenderModel::Mode renderMode, int32_t count) override;

    sp<Buffer> _buffer;
    sp<VKTexture> _texture;

private:
    void setupVertexDescriptions();
    void setupDescriptorSetLayout();
    void setupDescriptorPool();
    void setupDescriptorSet();

    void preparePipelines();

private:
    sp<GLResourceManager> _resource_manager;
    sp<RenderTarget> _render_target;
    sp<Device> _device;

    VkPipelineLayout pipelineLayout;
    VkDescriptorSetLayout descriptorSetLayout;

    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;

    struct {
        VkPipelineVertexInputStateCreateInfo inputState;
        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    } vertices;

    std::vector<VkCommandBuffer> _command_buffers;

    VkClearColorValue defaultClearColor = { { 0, 0, 0.2f, 1.0f } };
    struct {
        VkPipeline solid;
    } pipelines;

};

}
}

#endif

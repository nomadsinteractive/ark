#ifndef ARK_RENDERER_VULKAN_BASE_PIPELINE_FACTORY_H_
#define ARK_RENDERER_VULKAN_BASE_PIPELINE_FACTORY_H_

#include <vector>

#include <glm/glm.hpp>

#include <vulkan/vulkan.h>

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

#include "renderer/vulkan/forward.h"
#include "renderer/vulkan/base/vulkan_api.h"
#include "renderer/vulkan/util/vulkan_buffer.hpp"

namespace ark {
namespace vulkan {

class PipelineFactory {
public:
    PipelineFactory(const sp<GLResourceManager>& resourceManager, const sp<RenderTarget>& renderTarget);
    ~PipelineFactory();

    sp<Pipeline> build();

    sp<Buffer> _buffer;
    sp<Texture> _texture;

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

    struct {
        glm::mat4 projection;
        glm::mat4 model;
        glm::vec4 viewPos;
        float lodBias = 0.0f;
    } uboVS;

    std::vector<VkCommandBuffer> _command_buffers;

    VkClearColorValue defaultClearColor = { { 0, 0, 0.2f, 1.0f } };
    struct {
        VkPipeline solid;
    } pipelines;
};

}
}

#endif

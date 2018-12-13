#ifndef ARK_RENDERER_VULKAN_BASE_VULKAN_API_H_
#define ARK_RENDERER_VULKAN_BASE_VULKAN_API_H_

#include <vector>

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

#include "core/ark.h"
#include "core/types/shared_ptr.h"
#include "core/types/owned_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/vulkan/forward.h"
#include "renderer/vulkan/util/vulkan_device.hpp"
#include "renderer/vulkan/util/vulkan_swap_chain.hpp"

namespace ark {
namespace vulkan {

class VulkanAPI {
public:
    VulkanAPI(const sp<ResourceManager>& resourceManager);
    ~VulkanAPI();

    void initialize(GLContext& glContext);

    void render();

    static void checkResult(VkResult result);
    static VkPipelineShaderStageCreateInfo loadShaderSPIR(VkDevice device, std::string fileName, VkShaderStageFlagBits stage);

    struct Vertex {
        float pos[3];
        float uv[2];
        float normal[3];
    };

    struct {
        glm::mat4 projection;
        glm::mat4 model;
        glm::vec4 viewPos;
        float lodBias = 0.0f;
    } uboVS;

private:
    void generateQuad();

    void buildCommandBuffers();

    void updateUniformBuffers();

    void draw();

    uint32_t getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties) const;

private:
    sp<ResourceManager> _resource_manager;
    sp<VKInstance> _instance;
    sp<VKDevice> _device;
    sp<VKRenderTarget> _render_target;
    sp<PipelineFactoryVulkan> _pipeline_factory;
    sp<VKPipeline> _pipeline;
    sp<VKBuffer> _uniforms;

    std::vector<VkCommandBuffer> _command_buffers;

    float zoom = 0;

    glm::vec3 rotation = glm::vec3();
    glm::vec3 cameraPos = glm::vec3();

    std::vector<VkShaderModule> shaderModules;

//--------------------------------------------------------------------
    bool prepared = false;

    uint32_t indexCount;

    vks::Buffer vertexBuffer;
    vks::Buffer indexBuffer;

    VkClearColorValue defaultClearColor = { { 0, 0, 0.2f, 1.0f } };

    friend class RendererFactoryVulkan;
    friend class PipelineFactoryVulkan;
};

}
}

#endif

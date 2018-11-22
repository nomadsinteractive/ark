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
    VulkanAPI(const sp<GLResourceManager>& resourceManager);
    ~VulkanAPI();

    void initialize(GLContext& glContext);

    void render();

    static void checkResult(VkResult result);
    static VkPipelineShaderStageCreateInfo loadShader(VkDevice device, std::string fileName, VkShaderStageFlagBits stage);

    struct Vertex {
        float pos[3];
        float uv[2];
        float normal[3];
    };

    // For simplicity we use the same uniform block layout as in the shader:
    //
    //	layout(set = 0, binding = 0) uniform UBO
    //	{
    //		mat4 projectionMatrix;
    //		mat4 modelMatrix;
    //		mat4 viewMatrix;
    //	} ubo;
    //
    // This way we can just memcopy the ubo data to the ubo
    // Note: You should use data types that align with the GPU in order to avoid manual padding (vec4, mat4)
    struct {
        glm::mat4 projection;
        glm::mat4 model;
        glm::vec4 viewPos;
        float lodBias = 0.0f;
    } uboVS;

private:
    void generateQuad();
//    void loadTexture();

    void buildCommandBuffers();

    void updateUniformBuffers();

    void draw();

    uint32_t getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties) const;

private:
    sp<GLResourceManager> _resource_manager;
    sp<Instance> _instance;
    sp<Device> _device;
    sp<RenderTarget> _render_target;
    sp<PipelineFactory> _pipeline_factory;
    sp<Pipeline> _pipeline;
    sp<Buffer> _uniforms;

    VkDevice _logical_device;
    VkQueue _queue;

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
};

}
}

#endif

#ifndef ARK_RENDERER_VULKAN_BASE_VULKAN_API_H_
#define ARK_RENDERER_VULKAN_BASE_VULKAN_API_H_

#include <vector>

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

#include "core/ark.h"
#include "core/types/shared_ptr.h"
#include "core/types/owned_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/vulkan/base/vulkan_device.hpp"
#include "renderer/vulkan/base/vulkan_swap_chain.hpp"

namespace ark {
namespace vulkan {

class VulkanAPI {
public:
    VulkanAPI(const sp<GLResourceManager>& resourceManager);
    ~VulkanAPI();

    void initialize(GLContext& glContext);

    void render();

private:
    struct Vertex {
        float position[3];
        float color[3];
    };

    struct {
        VkDeviceMemory memory;															// Handle to the device memory for this buffer
        VkBuffer buffer;																// Handle to the Vulkan buffer object that the memory is bound to
    } vertices;

    struct
    {
        VkDeviceMemory memory;
        VkBuffer buffer;
        uint32_t count;
    } indices;

    // Uniform buffer block object
    struct {
        VkDeviceMemory memory;
        VkBuffer buffer;
        VkDescriptorBufferInfo descriptor;
    }  uniformBufferVS;

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
        glm::mat4 projectionMatrix;
        glm::mat4 modelMatrix;
        glm::mat4 viewMatrix;
    } uboVS;

    struct Texture {
        VkSampler sampler;
        VkImage image;
        VkImageLayout imageLayout;
        VkDeviceMemory deviceMemory;
        VkImageView view;
        uint32_t width, height;
        uint32_t mipLevels;
    } texture;

private:
    void createInstance();
    void createDevice();

    void initSwapchain();
    void createCommandPool();
    void createCommandBuffers();
    void setupDepthStencil();
    void setupRenderPass();
    void createPipelineCache();
    void setupFrameBuffer();

    void prepareVertices(bool useStagingBuffers);
    void prepareUniformBuffers();
    void updateUniformBuffers();
    void setupDescriptorSetLayout();
    void preparePipelines();
    void setupDescriptorPool();
    void setupDescriptorSet();
    void buildCommandBuffers();
    void generateQuad();

    void draw();

    void checkResult(VkResult result) const;

    uint32_t getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties) const;
    VkCommandBuffer getCommandBuffer(bool begin) const;
    void flushCommandBuffer(VkCommandBuffer commandBuffer) const;
    VkShaderModule loadSPIRVShader(std::string filename) const;

    void destroyCommandBuffers() const;
    void loadTexture();

    VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin) const;
    void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free) const;

    void destroyTextureImage(Texture texture) const;

private:
    sp<GLResourceManager> _resource_manager;

    std::vector<const char*> _instance_extensions;
    VkInstance _instance;
    VkPhysicalDevice _physical_device;

    // Stores physical device properties (for e.g. checking device limits)
    VkPhysicalDeviceProperties _device_properties;
    // Stores the features available on the selected physical device (for e.g. checking if a feature is available)
    VkPhysicalDeviceFeatures _device_features;
    // Stores all available memory (type) properties for the physical device
    VkPhysicalDeviceMemoryProperties _device_memory_properties;

    VkPhysicalDeviceFeatures _enabled_features {};
    /** @brief Set of device extensions to be enabled for this example (must be set in the derived constructor) */
    std::vector<const char*> _enabled_device_extensions;
    std::vector<const char*> _enabled_instance_extensions;

    op<vks::VulkanDevice> _vulkan_device;
    VkDevice _device;
    VkQueue _queue;
    // Depth buffer format (selected during Vulkan initialization)
    VkFormat _depth_format;

    VulkanSwapChain _swap_chain;

    VkSubmitInfo _submit_info;
    VkPipelineStageFlags _submit_pipeline_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkCommandPool _command_pool;

    std::vector<VkCommandBuffer> _command_buffers;

    VkRenderPass _render_pass;
    VkPipelineCache _pipeline_cache;

    std::vector<VkFramebuffer> _frame_buffers;

    struct {
        // Swap chain image presentation
        VkSemaphore present_complete;
        // Command buffer submission and execution
        VkSemaphore render_complete;
    } _semaphores;

    struct
    {
        VkImage image;
        VkDeviceMemory mem;
        VkImageView view;
    } _depth_stencil;

    uint32_t _width = 1280;
    uint32_t _height = 720;
    float zoom = 0;
    glm::vec3 rotation = glm::vec3();

//--------------------------------------------------------------------
    VkPipelineLayout pipelineLayout;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipeline _pipeline;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSet;
    bool prepared = false;
    uint32_t currentBuffer = 0;
};

}
}

#endif

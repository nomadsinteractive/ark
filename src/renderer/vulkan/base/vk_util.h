#ifndef ARK_RENDERER_VULKAN_BASE_VULKAN_API_H_
#define ARK_RENDERER_VULKAN_BASE_VULKAN_API_H_

#include <vector>

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

#include "core/ark.h"
#include "core/types/shared_ptr.h"
#include "core/types/owned_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/shader.h"

#include "renderer/vulkan/forward.h"
#include "renderer/vulkan/renderer_factory/renderer_factory_vulkan.h"
#include "renderer/vulkan/util/vulkan_device.hpp"
#include "renderer/vulkan/util/vulkan_swap_chain.hpp"

namespace ark {
namespace vulkan {

class VKUtil {
public:
    VKUtil(const sp<ResourceManager>& resourceManager, const sp<VKRenderer>& renderer);
    ~VKUtil();

    void initialize(GLContext& glContext);

    void render();

    static void checkResult(VkResult result);

    static VkPipelineShaderStageCreateInfo loadShaderSPIR(VkDevice device, std::string fileName, VkShaderStageFlagBits stage);
    static VkPipelineShaderStageCreateInfo loadShader(VkDevice device, const String& resid, Shader::Stage stage);
    static VkPipelineShaderStageCreateInfo createShader(VkDevice device, const String& source, Shader::Stage stage);


    static VkFormat getAttributeFormat(const Attribute& attribute);
    static VkFormat toTextureFormat(const Bitmap& bitmap, Texture::Format format);
    static VkShaderStageFlagBits toStage(Shader::Stage stage);

    static std::vector<uint32_t> compileSPIR(const String& source, Shader::Stage stage);

    struct Vertex {
        float pos[3];
        float uv[2];
        float normal[3];
    };

    struct UBO {
        glm::mat4 projection;
        glm::mat4 model;
        glm::vec4 viewPos;
        float lodBias = 0.0f;
    };

    sp<UBO> _ubo;

private:
    void generateQuad();

    void buildCommandBuffers(const VKRenderTarget& renderTarget);

    void updateUniformBuffers(const VKRenderTarget& renderTarget);

    void draw(VKRenderTarget& renderTarget);

    uint32_t getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties) const;

private:
    sp<ResourceManager> _resource_manager;
    sp<VKRenderer> _renderer;
    sp<PipelineFactoryVulkan> _pipeline_factory;
    sp<VKPipeline> _pipeline;
    sp<VKBuffer> _uniforms;

    sp<VKCommandBuffers> _command_buffers;

    float zoom = 0;

    glm::vec3 rotation = glm::vec3();
    glm::vec3 cameraPos = glm::vec3();

    std::vector<VkShaderModule> shaderModules;

//--------------------------------------------------------------------
    bool prepared = false;

    uint32_t indexCount;

    sp<VKBuffer> _vertex_buffer;
    sp<VKBuffer> _index_buffer;

    VkClearColorValue _background_color = { { 0, 0, 0.2f, 1.0f } };

    GraphicsContext _graphics_context;

    friend class RendererFactoryVulkan;
    friend class PipelineFactoryVulkan;
};

}
}

#endif

#ifndef ARK_RENDERER_VULKAN_BASE_VULKAN_API_H_
#define ARK_RENDERER_VULKAN_BASE_VULKAN_API_H_

#include <vector>

#include "core/ark.h"

#include "renderer/forwarding.h"
#include "renderer/base/shader.h"
#include "renderer/base/texture.h"

#include "renderer/vulkan/forward.h"

#include "platform/vulkan/vulkan.h"

namespace ark {
namespace vulkan {

class VKUtil {
public:
    static void checkResult(VkResult result);

    static VkPipelineShaderStageCreateInfo loadShaderSPIR(VkDevice device, std::string fileName, VkShaderStageFlagBits stage);
    static VkPipelineShaderStageCreateInfo loadShader(VkDevice device, const String& resid, Shader::Stage stage);
    static VkPipelineShaderStageCreateInfo createShader(VkDevice device, const String& source, Shader::Stage stage);


    static VkFormat getAttributeFormat(const Attribute& attribute);
    static VkFormat toTextureFormat(const Bitmap& bitmap, Texture::Format format);
    static VkShaderStageFlagBits toStage(Shader::Stage stage);
    static VkPrimitiveTopology toPrimitiveTopology(RenderModel::Mode mode);

    static std::vector<uint32_t> compileSPIR(const String& source, Shader::Stage stage);

};

}
}

#endif
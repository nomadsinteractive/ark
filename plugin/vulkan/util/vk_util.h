#pragma once

#include <vector>

#include "core/ark.h"

#include "renderer/forwarding.h"
#include "renderer/base/attribute.h"
#include "renderer/base/shader.h"
#include "renderer/base/texture.h"

#include "vulkan/forwarding.h"

#include "platform/vulkan/vulkan.h"

namespace ark::plugin::vulkan {

class VKUtil {
public:
    static void checkResult(VkResult result);

    static VkPipelineShaderStageCreateInfo loadShader(VkDevice device, const String& resid, Enum::ShaderStageBit stage);
    static VkPipelineShaderStageCreateInfo createShader(VkDevice device, const String& source, Enum::ShaderStageBit stage);

    static void createImage(const VKDevice& device, const VkImageCreateInfo& imageCreateInfo, VkImage* image, VkDeviceMemory* memory, VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    static VkImageLayout toImageLayout(Texture::Usage usage);
    static VkImageLayout toAttachmentImageLayout(Texture::Usage usage);

    static VkFormat toAttributeFormat(Attribute::Type type, uint32_t length);
    static VkFormat toTextureFormat(uint32_t depths, uint8_t channels, Texture::Format format);
    static VkFormat toTextureFormat(const Bitmap& bitmap, Texture::Format format);
    static VkFormat toTextureFormat(Texture::Format format);
    static VkFrontFace toFrontFace(PipelineDescriptor::FrontFace frontFace);
    static VkCompareOp toCompareOp(PipelineDescriptor::CompareFunc func);
    static VkStencilOp toStencilOp(PipelineDescriptor::StencilFunc func);

    static VkImageAspectFlags toTextureAspect(Texture::Usage usage);
    static VkShaderStageFlagBits toStage(Enum::ShaderStageBit stage);
    static VkPrimitiveTopology toPrimitiveTopology(Enum::RenderMode mode);

};

}

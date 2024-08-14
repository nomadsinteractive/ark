#include "renderer/vulkan/util/vk_util.h"

#include <array>

#include <glm/gtc/matrix_transform.hpp>

#include "core/base/plugin_manager.h"
#include "core/inf/variable.h"
#include "core/types/global.h"
#include "core/util/log.h"

#include "graphics/forwarding.h"
#include "graphics/base/bitmap.h"

#include "renderer/base/attribute.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/util/render_util.h"

#include "renderer/vulkan/base/vk_buffer.h"
#include "renderer/vulkan/base/vk_command_pool.h"
#include "renderer/vulkan/base/vk_command_buffers.h"
#include "renderer/vulkan/base/vk_device.h"
#include "renderer/vulkan/base/vk_instance.h"
#include "renderer/vulkan/base/vk_pipeline.h"
#include "renderer/vulkan/base/vk_renderer.h"
#include "renderer/vulkan/base/vk_swap_chain.h"
#include "renderer/vulkan/pipeline_factory/pipeline_factory_vulkan.h"
#include "renderer/vulkan/util/vulkan_tools.h"
#include "renderer/vulkan/util/vulkan_debug.h"

#define VERTEX_BUFFER_BIND_ID 0

namespace ark::vulkan {

void VKUtil::checkResult(VkResult result)
{
    CHECK(result == VK_SUCCESS, "Vulkan error: %s", vks::tools::errorString(result).c_str());
}

VkPipelineShaderStageCreateInfo VKUtil::loadShaderSPIR(VkDevice device, std::string fileName, VkShaderStageFlagBits stage)
{
    VkPipelineShaderStageCreateInfo shaderStage = {};
    shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.stage = stage;
    shaderStage.module = vks::tools::loadShader(fileName.c_str(), device);
    shaderStage.pName = "main";
    DASSERT(shaderStage.module != VK_NULL_HANDLE);
    return shaderStage;
}

VkPipelineShaderStageCreateInfo VKUtil::loadShader(VkDevice device, const String& resid, PipelineInput::ShaderStage stage)
{
    const String content = Strings::loadFromReadable(Ark::instance().openAsset(resid));
    return createShader(device, content, stage);
}

VkPipelineShaderStageCreateInfo VKUtil::createShader(VkDevice device, const String& source, PipelineInput::ShaderStage stage)
{
    const std::vector<uint32_t> spirv = RenderUtil::compileSPIR(source, stage, Ark::RENDERER_TARGET_VULKAN);
    VkShaderModuleCreateInfo moduleCreateInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    moduleCreateInfo.codeSize = spirv.size() * sizeof(uint32_t);
    moduleCreateInfo.pCode = spirv.data();

    VkPipelineShaderStageCreateInfo shaderStage = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    shaderStage.stage = toStage(stage);
    checkResult(vkCreateShaderModule(device, &moduleCreateInfo, nullptr, &shaderStage.module));
    DASSERT(shaderStage.module != VK_NULL_HANDLE);
    shaderStage.pName = "main";
    return shaderStage;
}

void VKUtil::createImage(const VKDevice& device, const VkImageCreateInfo& imageCreateInfo, VkImage* image, VkDeviceMemory* memory, VkMemoryPropertyFlags propertyFlags)
{
    VkDevice logicalDevice = device.vkLogicalDevice();
    checkResult(vkCreateImage(logicalDevice, &imageCreateInfo, nullptr, image));

    VkMemoryRequirements memReqs = {};
    VkMemoryAllocateInfo memAllocInfo = vks::initializers::memoryAllocateInfo();
    vkGetImageMemoryRequirements(logicalDevice, *image, &memReqs);
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = device.getMemoryType(memReqs.memoryTypeBits, propertyFlags);
    checkResult(vkAllocateMemory(logicalDevice, &memAllocInfo, nullptr, memory));
    checkResult(vkBindImageMemory(logicalDevice, *image, *memory, 0));
}

VkFormat VKUtil::toAttributeFormat(Attribute::Type type, uint32_t length)
{
    if(length > 0 && length < 5)
    {
        switch(type)
        {
            case Attribute::TYPE_FLOAT:
            {
                constexpr VkFormat formats[4] = {VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT};
                return formats[length - 1];
            }
            case Attribute::TYPE_BYTE:
            case Attribute::TYPE_UBYTE:
            {
                constexpr VkFormat formats[4] = {VK_FORMAT_R8_UNORM, VK_FORMAT_R8G8_UNORM, VK_FORMAT_R8G8B8_UNORM, VK_FORMAT_R8G8B8A8_UNORM};
                return formats[length - 1];
            }
            case Attribute::TYPE_INTEGER:
            {
                constexpr VkFormat formats[4] = {VK_FORMAT_R32_SINT, VK_FORMAT_R32G32_SINT, VK_FORMAT_R32G32B32_SINT, VK_FORMAT_R32G32B32A32_SINT};
                return formats[length - 1];
            }
            case Attribute::TYPE_SHORT:
            {
                constexpr VkFormat formats[4] = {VK_FORMAT_R16_SNORM, VK_FORMAT_R16G16_SNORM, VK_FORMAT_R16G16B16_SNORM, VK_FORMAT_R16G16B16A16_SNORM};
                return formats[length - 1];
            }
            case Attribute::TYPE_USHORT:
            {
                constexpr VkFormat formats[4] = {VK_FORMAT_R16_UNORM, VK_FORMAT_R16G16_UNORM, VK_FORMAT_R16G16B16_UNORM, VK_FORMAT_R16G16B16A16_UNORM};
                return formats[length - 1];
            }
            default:
                break;
        }
    }
    DFATAL("Unsupport type %d, length %d", type, length);
    return VK_FORMAT_R32G32B32A32_SFLOAT;
}

static VkFormat toVkChannelFormat(const VkFormat* channelFormat, uint32_t componentSize, Texture::Format format)
{
/*
 *    VK_FORMAT_R8G8_UNORM, VK_FORMAT_R8G8_SNORM,
      VK_FORMAT_R16G16_UNORM, VK_FORMAT_R16G16_SNORM,
      VK_FORMAT_R16G16_SFLOAT, VK_FORMAT_R32G32_SFLOAT,
      VK_FORMAT_R32G32_UINT, VK_FORMAT_R32G32_SINT
*/
    if(componentSize == 1)
    {
        CHECK(!(format & Texture::FORMAT_FLOAT), "Component size one doesn't support float format");
        return channelFormat[0];
    }
    if(componentSize == 2)
    {
        if(format & Texture::FORMAT_FLOAT)
            return channelFormat[4];
        return format & Texture::FORMAT_SIGNED ? channelFormat[3] : channelFormat[2];
    }
    DCHECK(componentSize == 4, "Unsupported color-depth: %d", componentSize * 8);
    if(format & Texture::FORMAT_FLOAT)
        return channelFormat[5];
    return format & Texture::FORMAT_SIGNED ? channelFormat[7] : channelFormat[6];
}

VkFormat VKUtil::toTextureFormat(uint32_t componentSize, uint8_t channels, Texture::Format format)
{
    static const VkFormat vkFormats[] = {
        VK_FORMAT_R8_UNORM, VK_FORMAT_R8_SNORM, VK_FORMAT_R16_UNORM, VK_FORMAT_R16_SNORM, VK_FORMAT_R16_SFLOAT, VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32_UINT, VK_FORMAT_R32_SINT,
        VK_FORMAT_R8G8_UNORM, VK_FORMAT_R8G8_SNORM, VK_FORMAT_R16G16_UNORM, VK_FORMAT_R16G16_SNORM, VK_FORMAT_R16G16_SFLOAT, VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32_UINT, VK_FORMAT_R32G32_SINT,
        VK_FORMAT_R8G8B8_UNORM, VK_FORMAT_R8G8B8_SNORM, VK_FORMAT_R16G16B16_UNORM, VK_FORMAT_R16G16B16_SNORM, VK_FORMAT_R16G16B16_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32_UINT, VK_FORMAT_R32G32B32_SINT,
        VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SNORM, VK_FORMAT_R16G16B16A16_UNORM, VK_FORMAT_R16G16B16A16_SNORM, VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_R32G32B32A32_UINT, VK_FORMAT_R32G32B32A32_SINT
    };
    CHECK(!(format & Texture::FORMAT_SIGNED && format & Texture::FORMAT_FLOAT), "FORMAT_SIGNED format can not combined with FORMAT_FLOAT");
    uint32_t channel8 = (channels - 1) * 8;
    CHECK_WARN(channels != 3, "RGB texture format may not be supported by all the graphics drivers");
    return toVkChannelFormat(vkFormats + channel8, componentSize, format);
}

VkFormat VKUtil::toTextureFormat(const Bitmap& bitmap, Texture::Format format)
{
    return toTextureFormat(bitmap.rowBytes() / bitmap.width() / bitmap.channels(), bitmap.channels(), format);
}

VkFormat VKUtil::toTextureFormat(Texture::Format format)
{
    DCHECK(format != Texture::FORMAT_AUTO, "Cannot determine texture format(auto) without a bitmap");
    return toTextureFormat(RenderUtil::getComponentSize(format), (format & Texture::FORMAT_RGBA) + 1, format);
}

VkFrontFace VKUtil::toFrontFace(PipelineDescriptor::FrontFace frontFace)
{
    switch(frontFace) {
        case PipelineDescriptor::FRONT_FACE_DEFAULT:
        case PipelineDescriptor::FRONT_FACE_COUTER_CLOCK_WISE:
            return VK_FRONT_FACE_COUNTER_CLOCKWISE;
        case PipelineDescriptor::FRONT_FACE_CLOCK_WISE:
            return VK_FRONT_FACE_CLOCKWISE;
        default:
            DFATAL("Unknow front face: %d", frontFace);
    }
    return VK_FRONT_FACE_COUNTER_CLOCKWISE;
}

VkCompareOp VKUtil::toCompareOp(PipelineDescriptor::CompareFunc func)
{
    const VkCompareOp compareOps[] = {VK_COMPARE_OP_LESS_OR_EQUAL, VK_COMPARE_OP_ALWAYS, VK_COMPARE_OP_NEVER, VK_COMPARE_OP_EQUAL, VK_COMPARE_OP_NOT_EQUAL, VK_COMPARE_OP_LESS,
                                      VK_COMPARE_OP_GREATER, VK_COMPARE_OP_LESS_OR_EQUAL, VK_COMPARE_OP_GREATER_OR_EQUAL};
    DASSERT(func < PipelineDescriptor::COMPARE_FUNC_LENGTH);
    return compareOps[func];
}

VkStencilOp VKUtil::toStencilOp(PipelineDescriptor::StencilFunc func)
{
    const VkStencilOp stencilOps[] = {VK_STENCIL_OP_KEEP, VK_STENCIL_OP_ZERO, VK_STENCIL_OP_REPLACE, VK_STENCIL_OP_INCREMENT_AND_CLAMP, VK_STENCIL_OP_INCREMENT_AND_WRAP,
                                      VK_STENCIL_OP_DECREMENT_AND_CLAMP, VK_STENCIL_OP_DECREMENT_AND_WRAP, VK_STENCIL_OP_INVERT};
    DASSERT(func < PipelineDescriptor::STENCIL_FUNC_LENGTH);
    return stencilOps[func];
}

VkImageUsageFlags VKUtil::toTextureUsage(Texture::Usage usage)
{
    VkImageUsageFlags vkFlags = 0;
    if(usage == Texture::USAGE_GENERAL)
        vkFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if(usage & Texture::USAGE_DEPTH_STENCIL_ATTACHMENT)
        vkFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    return vkFlags;
}

VkImageAspectFlags VKUtil::toTextureAspect(Texture::Usage usage)
{
    VkImageAspectFlags vkFlags = 0;
    if(usage == Texture::USAGE_GENERAL)
        vkFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    if(usage & Texture::USAGE_DEPTH_ATTACHMENT)
        vkFlags |= VK_IMAGE_ASPECT_DEPTH_BIT;
    if(usage & Texture::USAGE_STENCIL_ATTACHMENT)
        vkFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
    return vkFlags;
}

VkShaderStageFlagBits VKUtil::toStage(PipelineInput::ShaderStage stage)
{
#ifndef ANDROID
    constexpr VkShaderStageFlagBits vkStages[PipelineInput::SHADER_STAGE_COUNT] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                                                                                   VK_SHADER_STAGE_GEOMETRY_BIT, VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_STAGE_COMPUTE_BIT};
#else
    constexpr VkShaderStageFlagBits vkStages[PipelineInput::SHADER_STAGE_COUNT] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_STAGE_COMPUTE_BIT};
#endif
    DCHECK(stage > PipelineInput::SHADER_STAGE_NONE && stage < PipelineInput::SHADER_STAGE_COUNT, "Illegal PipelineInput::ShaderStage: %d", stage);
    return vkStages[stage];
}

VkPrimitiveTopology VKUtil::toPrimitiveTopology(Enum::RenderMode mode)
{
    constexpr VkPrimitiveTopology topologies[Enum::RENDER_MODE_COUNT] = {VK_PRIMITIVE_TOPOLOGY_LINE_LIST, VK_PRIMITIVE_TOPOLOGY_POINT_LIST, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP};
    CHECK(mode >= 0 && mode < Enum::RENDER_MODE_COUNT, "Unsupported render-mode: %d", mode);
    return topologies[mode];
}

}

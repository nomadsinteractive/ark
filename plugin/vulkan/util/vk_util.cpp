#include "vulkan/util/vk_util.h"

#include "core/base/plugin_manager.h"
#include "core/util/log.h"

#include "graphics/forwarding.h"
#include "graphics/base/bitmap.h"

#include "renderer/base/attribute.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/util/render_util.h"

#include "vulkan/base/vk_command_pool.h"
#include "vulkan/base/vk_device.h"
#include "vulkan/base/vk_pipeline.h"
#include "vulkan/util/vulkan_tools.h"
#include "vulkan/util/vulkan_debug.h"

#define VERTEX_BUFFER_BIND_ID 0

namespace ark::plugin::vulkan {

namespace {

bool isDepthFormatSupported(VkPhysicalDevice physicalDevice, VkFormat format)
{
    VkFormatProperties formatProps;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
    return formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
}


VkFormat toVkChannelFormat(const VkFormat* channelFormat, const uint32_t depths, const Texture::Format format)
{
    if(depths == 1)
    {
        CHECK(!(format & Texture::FORMAT_FLOAT), "Component size one doesn't support float format");
        return channelFormat[0];
    }
    if(depths == 2)
    {
        if(format & Texture::FORMAT_FLOAT)
            return channelFormat[4];
        return format & Texture::FORMAT_SIGNED ? channelFormat[3] : channelFormat[2];
    }
    DCHECK(depths == 4, "Unsupported color-depth: %d", depths * 8);
    if(format & Texture::FORMAT_FLOAT)
        return channelFormat[5];
    return format & Texture::FORMAT_SIGNED ? channelFormat[7] : channelFormat[6];
}

}

void VKUtil::checkResult(const VkResult result)
{
    CHECK(result == VK_SUCCESS, "Vulkan error: %s", vks::tools::errorString(result).c_str());
}

VkPipelineShaderStageCreateInfo VKUtil::loadShader(const VkDevice device, const String& resid, enums::ShaderStageBit stage)
{
    const String content = Strings::loadFromReadable(Ark::instance().openAsset(resid));
    return createShader(device, content, stage);
}

VkPipelineShaderStageCreateInfo VKUtil::createShader(const VkDevice device, const String& source, enums::ShaderStageBit stage)
{
    const Vector<uint32_t> spirv = RenderUtil::compileSPIR(source, stage, enums::RENDERING_BACKEND_BIT_VULKAN);
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

VkImageLayout VKUtil::toImageLayout(const Texture::Usage usage)
{
    if(usage.has(Texture::USAGE_ATTACHMENT))
        return toAttachmentImageLayout(usage);

    if(usage.has(Texture::USAGE_SAMPLER))
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    return VK_IMAGE_LAYOUT_GENERAL;
}

VkImageLayout VKUtil::toAttachmentImageLayout(const Texture::Usage usage)
{
    switch(usage.bits() & Texture::USAGE_DEPTH_STENCIL_ATTACHMENT)
    {
        case Texture::USAGE_DEPTH_ATTACHMENT:
            return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        case Texture::USAGE_STENCIL_ATTACHMENT:
            return VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        case Texture::USAGE_DEPTH_STENCIL_ATTACHMENT:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        default:
            break;
    }
    if(usage.has(Texture::USAGE_SAMPLER) || usage.has(Texture::USAGE_STORAGE))
        return VK_IMAGE_LAYOUT_GENERAL;
    return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
}

VkFormat VKUtil::toAttributeFormat(const Attribute::Type type, const uint32_t length)
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
            case Attribute::TYPE_INT:
            {
                constexpr VkFormat formats[4] = {VK_FORMAT_R32_SINT, VK_FORMAT_R32G32_SINT, VK_FORMAT_R32G32B32_SINT, VK_FORMAT_R32G32B32A32_SINT};
                return formats[length - 1];
            }
            case Attribute::TYPE_UINT:
            {
                constexpr VkFormat formats[4] = {VK_FORMAT_R32_UINT, VK_FORMAT_R32G32_UINT, VK_FORMAT_R32G32B32_UINT, VK_FORMAT_R32G32B32A32_UINT};
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

VkFormat VKUtil::toTextureFormat(const uint32_t depths, const uint8_t channels, const Texture::Format format)
{
    constexpr VkFormat vkFormats[] = {
        VK_FORMAT_R8_UNORM, VK_FORMAT_R8_SNORM, VK_FORMAT_R16_UNORM, VK_FORMAT_R16_SNORM, VK_FORMAT_R16_SFLOAT, VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32_UINT, VK_FORMAT_R32_SINT,
        VK_FORMAT_R8G8_UNORM, VK_FORMAT_R8G8_SNORM, VK_FORMAT_R16G16_UNORM, VK_FORMAT_R16G16_SNORM, VK_FORMAT_R16G16_SFLOAT, VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32_UINT, VK_FORMAT_R32G32_SINT,
        VK_FORMAT_R8G8B8_UNORM, VK_FORMAT_R8G8B8_SNORM, VK_FORMAT_R16G16B16_UNORM, VK_FORMAT_R16G16B16_SNORM, VK_FORMAT_R16G16B16_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32_UINT, VK_FORMAT_R32G32B32_SINT,
        VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SNORM, VK_FORMAT_R16G16B16A16_UNORM, VK_FORMAT_R16G16B16A16_SNORM, VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_R32G32B32A32_UINT, VK_FORMAT_R32G32B32A32_SINT
    };
    CHECK(!(format & Texture::FORMAT_SIGNED && format & Texture::FORMAT_FLOAT), "FORMAT_SIGNED format can not combined with FORMAT_FLOAT");
    const uint32_t channel8 = (channels - 1) * 8;
    CHECK_WARN(channels != 3, "RGB texture format may not be supported by all the graphics drivers");
    return toVkChannelFormat(vkFormats + channel8, depths, format);
}

VkFormat VKUtil::toTextureFormat(const Bitmap& bitmap, const Texture::Format format)
{
    return toTextureFormat(bitmap.depth(), bitmap.channels(), format);
}

VkFormat VKUtil::toTextureFormat(const Texture::Format format)
{
    DCHECK(format != Texture::FORMAT_AUTO, "Cannot determine texture format(auto) without a bitmap");
    return toTextureFormat(RenderUtil::getComponentSize(format), (format & Texture::FORMAT_RGBA) + 1, format);
}

VkFrontFace VKUtil::toFrontFace(const PipelineDescriptor::FrontFace frontFace)
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

VkCompareOp VKUtil::toCompareOp(const PipelineDescriptor::CompareFunc func)
{
    constexpr VkCompareOp compareOps[] = {VK_COMPARE_OP_LESS_OR_EQUAL, VK_COMPARE_OP_ALWAYS, VK_COMPARE_OP_NEVER, VK_COMPARE_OP_EQUAL, VK_COMPARE_OP_NOT_EQUAL, VK_COMPARE_OP_LESS,
                                          VK_COMPARE_OP_GREATER, VK_COMPARE_OP_LESS_OR_EQUAL, VK_COMPARE_OP_GREATER_OR_EQUAL};
    DASSERT(func < PipelineDescriptor::COMPARE_FUNC_LENGTH);
    return compareOps[func];
}

VkStencilOp VKUtil::toStencilOp(const PipelineDescriptor::StencilFunc func)
{
    constexpr VkStencilOp stencilOps[] = {VK_STENCIL_OP_KEEP, VK_STENCIL_OP_ZERO, VK_STENCIL_OP_REPLACE, VK_STENCIL_OP_INCREMENT_AND_CLAMP, VK_STENCIL_OP_INCREMENT_AND_WRAP,
                                          VK_STENCIL_OP_DECREMENT_AND_CLAMP, VK_STENCIL_OP_DECREMENT_AND_WRAP, VK_STENCIL_OP_INVERT};
    DASSERT(func < PipelineDescriptor::STENCIL_FUNC_LENGTH);
    return stencilOps[func];
}

VkImageAspectFlags VKUtil::toTextureAspect(const Texture::Usage usage)
{
    VkImageAspectFlags vkFlags = usage.has(Texture::USAGE_DEPTH_STENCIL_ATTACHMENT) ? 0 : VK_IMAGE_ASPECT_COLOR_BIT;
    if(usage.has(Texture::USAGE_DEPTH_ATTACHMENT))
        vkFlags |= VK_IMAGE_ASPECT_DEPTH_BIT;
    if(usage.has(Texture::USAGE_STENCIL_ATTACHMENT))
        vkFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
    return vkFlags;
}

VkShaderStageFlagBits VKUtil::toStage(enums::ShaderStageBit stage)
{
#ifndef ANDROID
    constexpr VkShaderStageFlagBits vkStages[enums::SHADER_STAGE_BIT_COUNT] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                                                                                   VK_SHADER_STAGE_GEOMETRY_BIT, VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_STAGE_COMPUTE_BIT};
#else
    constexpr VkShaderStageFlagBits vkStages[Enum::SHADER_STAGE_COUNT] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_STAGE_COMPUTE_BIT};
#endif
    DCHECK(stage > enums::SHADER_STAGE_BIT_NONE && stage < enums::SHADER_STAGE_BIT_COUNT, "Illegal ShaderStage::BitSet: %d", stage);
    return vkStages[stage];
}

VkPrimitiveTopology VKUtil::toPrimitiveTopology(enums::DrawMode mode)
{
    constexpr VkPrimitiveTopology topologies[enums::DRAW_MODE_COUNT] = {VK_PRIMITIVE_TOPOLOGY_LINE_LIST, VK_PRIMITIVE_TOPOLOGY_POINT_LIST, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP};
    CHECK(mode >= 0 && mode < enums::DRAW_MODE_COUNT, "Unsupported render-mode: %d", mode);
    return topologies[mode];
}

VkFormat findSupportedDepthFormat(const VkPhysicalDevice physicalDevice, const VkFormat choice)
{
    if(isDepthFormatSupported(physicalDevice, choice))
        return choice;
    return VK_FORMAT_UNDEFINED;
}

VkFormat VKUtil::getSupportedDepthFormat(const VkPhysicalDevice physicalDevice, const Texture::Format format, const Texture::Usage usage)
{
    const bool hasStencil = usage.has(Texture::USAGE_STENCIL_ATTACHMENT);
    VkFormat depthFormat = VK_FORMAT_UNDEFINED;
    switch(format & Texture::FORMAT_BIT_MASK)
    {
        case Texture::FORMAT_16_BIT:
            depthFormat = hasStencil ? VK_FORMAT_D16_UNORM_S8_UINT : VK_FORMAT_D16_UNORM;
            break;
        case Texture::FORMAT_24_BIT:
            depthFormat = hasStencil ? VK_FORMAT_D24_UNORM_S8_UINT : VK_FORMAT_UNDEFINED;
            break;
        case Texture::FORMAT_32_BIT:
            depthFormat = hasStencil ? VK_FORMAT_D32_SFLOAT_S8_UINT : VK_FORMAT_D32_SFLOAT;
            break;
        default:
            break;
    }
    CHECK(depthFormat != VK_FORMAT_UNDEFINED && isDepthFormatSupported(physicalDevice, depthFormat), "Unsupported depth texture format: %d", format);
    return depthFormat;
}

}

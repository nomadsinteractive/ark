#include "renderer/vulkan/base/vk_texture_2d.h"

#include "core/base/string.h"
#include "core/inf/dictionary.h"
#include "core/inf/variable.h"

#include "graphics/base/bitmap.h"

#include "renderer/base/resource_manager.h"
#include "renderer/base/recycler.h"

#include "renderer/vulkan/base/vk_device.h"
#include "renderer/vulkan/base/vk_command_pool.h"
#include "renderer/vulkan/base/vk_renderer.h"
#include "renderer/vulkan/base/vk_util.h"

namespace ark {
namespace vulkan {

VKTexture2D::VKTexture2D(const sp<Recycler>& recycler, const sp<VKRenderer>& renderer, const sp<Texture::Parameters>& parameters, const sp<Variable<bitmap>>& bitmap)
    : _recycler(recycler), _renderer(renderer), _parameters(parameters), _bitmap(bitmap), _image(VK_NULL_HANDLE), _memory(VK_NULL_HANDLE), _descriptor{}
{
}

VKTexture2D::~VKTexture2D()
{
    _recycler->recycle(*this);
}

uint64_t VKTexture2D::id()
{
    return static_cast<uint64_t>(_image);
}

void VKTexture2D::upload(GraphicsContext& /*graphicsContext*/)
{
    doUpload();
}

Resource::RecycleFunc VKTexture2D::recycle()
{
    const sp<VKDevice> device = _renderer->device();
    VkDescriptorImageInfo descriptor = _descriptor;
    VkImage image = _image;
    VkDeviceMemory memory = _memory;

    _image = VK_NULL_HANDLE;
    _memory = VK_NULL_HANDLE;

    return [device, descriptor, image, memory](GraphicsContext&) {
        vkDestroyImageView(device->logicalDevice(), descriptor.imageView, nullptr);
        vkDestroyImage(device->logicalDevice(), image, nullptr);
        vkDestroySampler(device->logicalDevice(), descriptor.sampler, nullptr);
        vkFreeMemory(device->logicalDevice(), memory, nullptr);
    };
}

const VkDescriptorImageInfo& VKTexture2D::vkDescriptor() const
{
    return _descriptor;
}

void VKTexture2D::doUpload()
{
    VkDevice logicalDevice = _renderer->vkLogicalDevice();

    const bitmap tex = _bitmap->val();
    VkFormat format = VKUtil::toTextureFormat(tex, _parameters->_format);

    _width = tex->width();
    _height = tex->height();
    _mip_levels = 1;

    // We prefer using staging to copy the texture data to a device local optimal image
    VkBool32 useStaging = true;

    // Only use linear tiling if forced
    bool forceLinearTiling = false;
    if (forceLinearTiling) {
        // Don't use linear if format is not supported for (linear) shader sampling
        // Get device properites for the requested texture format
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(_renderer->vkPhysicalDevice(), format, &formatProperties);
        useStaging = !(formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
    }

    VkMemoryAllocateInfo memAllocInfo = vks::initializers::memoryAllocateInfo();
    VkMemoryRequirements memReqs = {};

    if (useStaging) {
        // Copy data to an optimal tiled image
        // This loads the texture data into a host local buffer that is copied to the optimal tiled image on the device

        // Create a host-visible staging buffer that contains the raw image data
        // This buffer will be the data source for copying texture data to the optimal tiled image on the device
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingMemory;

        VkBufferCreateInfo bufferCreateInfo = vks::initializers::bufferCreateInfo();
        bufferCreateInfo.size = tex->bytes()->size();
        // This buffer is used as a transfer source for the buffer copy
        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VKUtil::checkResult(vkCreateBuffer(logicalDevice, &bufferCreateInfo, nullptr, &stagingBuffer));

        // Get memory requirements for the staging buffer (alignment, memory type bits)
        vkGetBufferMemoryRequirements(logicalDevice, stagingBuffer, &memReqs);
        memAllocInfo.allocationSize = memReqs.size;
        // Get memory type index for a host visible buffer
        memAllocInfo.memoryTypeIndex = _renderer->device()->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        VKUtil::checkResult(vkAllocateMemory(logicalDevice, &memAllocInfo, nullptr, &stagingMemory));
        VKUtil::checkResult(vkBindBufferMemory(logicalDevice, stagingBuffer, stagingMemory, 0));

        // Copy texture data into host local staging buffer
        uint8_t *data;
        VKUtil::checkResult(vkMapMemory(logicalDevice, stagingMemory, 0, memReqs.size, 0, (void **)&data));
        memcpy(data, tex->bytes()->buf(), tex->bytes()->size());
        vkUnmapMemory(logicalDevice, stagingMemory);

        // Setup buffer copy regions for each mip level
        std::vector<VkBufferImageCopy> bufferCopyRegions;
        uint32_t offset = 0;

        for (uint32_t i = 0; i < _mip_levels; i++) {
            VkBufferImageCopy bufferCopyRegion = {};
            bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            bufferCopyRegion.imageSubresource.mipLevel = i;
            bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
            bufferCopyRegion.imageSubresource.layerCount = 1;
            bufferCopyRegion.imageExtent.width = tex->width();
            bufferCopyRegion.imageExtent.height = tex->height();
            bufferCopyRegion.imageExtent.depth = 1;
            bufferCopyRegion.bufferOffset = offset;

            bufferCopyRegions.push_back(bufferCopyRegion);

            offset += static_cast<uint32_t>(tex->bytes()->size());
        }

        // Create optimal tiled target image on the device
        VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = format;
        imageCreateInfo.mipLevels = _mip_levels;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        // Set initial layout of the image to undefined
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.extent = { _width, _height, 1 };
        imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        VKUtil::checkResult(vkCreateImage(logicalDevice, &imageCreateInfo, nullptr, &_image));

        vkGetImageMemoryRequirements(logicalDevice, _image, &memReqs);
        memAllocInfo.allocationSize = memReqs.size;
        memAllocInfo.memoryTypeIndex = _renderer->device()->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VKUtil::checkResult(vkAllocateMemory(logicalDevice, &memAllocInfo, nullptr, &_memory));
        VKUtil::checkResult(vkBindImageMemory(logicalDevice, _image, _memory, 0));

        VkCommandBuffer copyCmd = _renderer->commandPool()->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

        // Image memory barriers for the texture image

        // The sub resource range describes the regions of the image that will be transitioned using the memory barriers below
        VkImageSubresourceRange subresourceRange = {};
        // Image only contains color data
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        // Start at first mip level
        subresourceRange.baseMipLevel = 0;
        // We will transition on all mip levels
        subresourceRange.levelCount = _mip_levels;
        // The 2D texture only has one layer
        subresourceRange.layerCount = 1;

        // Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
        VkImageMemoryBarrier imageMemoryBarrier = vks::initializers::imageMemoryBarrier();;
        imageMemoryBarrier.image = _image;
        imageMemoryBarrier.subresourceRange = subresourceRange;
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

        // Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
        // Source pipeline stage is host write/read exection (VK_PIPELINE_STAGE_HOST_BIT)
        // Destination pipeline stage is copy command exection (VK_PIPELINE_STAGE_TRANSFER_BIT)
        vkCmdPipelineBarrier(
                    copyCmd,
                    VK_PIPELINE_STAGE_HOST_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &imageMemoryBarrier);

        // Copy mip levels from staging buffer
        vkCmdCopyBufferToImage(
                    copyCmd,
                    stagingBuffer,
                    _image,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    static_cast<uint32_t>(bufferCopyRegions.size()),
                    bufferCopyRegions.data());

        // Once the data has been uploaded we transfer to the texture image to the shader read layout, so it can be sampled from
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
        // Source pipeline stage stage is copy command exection (VK_PIPELINE_STAGE_TRANSFER_BIT)
        // Destination pipeline stage fragment shader access (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
        vkCmdPipelineBarrier(
                    copyCmd,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &imageMemoryBarrier);

        // Store current layout for later reuse
        _descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        _renderer->commandPool()->flushCommandBuffer(copyCmd, true);

        // Clean up staging resources
        vkFreeMemory(logicalDevice, stagingMemory, nullptr);
        vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
    } else {
        // Copy data to a linear tiled image

        VkImage mappableImage;
        VkDeviceMemory mappableMemory;

        // Load mip map level 0 to linear tiling image
        VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = format;
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
        imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        imageCreateInfo.extent = { _width, _height, 1 };
        VKUtil::checkResult(vkCreateImage(logicalDevice, &imageCreateInfo, nullptr, &mappableImage));

        // Get memory requirements for this image like size and alignment
        vkGetImageMemoryRequirements(logicalDevice, mappableImage, &memReqs);
        // Set memory allocation size to required memory size
        memAllocInfo.allocationSize = memReqs.size;
        // Get memory type that can be mapped to host memory
        memAllocInfo.memoryTypeIndex = _renderer->device()->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        VKUtil::checkResult(vkAllocateMemory(logicalDevice, &memAllocInfo, nullptr, &mappableMemory));
        VKUtil::checkResult(vkBindImageMemory(logicalDevice, mappableImage, mappableMemory, 0));

        // Map image memory
        void *data;
        VKUtil::checkResult(vkMapMemory(logicalDevice, mappableMemory, 0, memReqs.size, 0, &data));
        // Copy image data of the first mip level into memory
        memcpy(data, tex->bytes()->buf(), tex->bytes()->size());
        vkUnmapMemory(logicalDevice, mappableMemory);

        // Linear tiled images don't need to be staged and can be directly used as textures
        _image = mappableImage;
        _memory = mappableMemory;
        _descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // Setup image memory barrier transfer image to shader read layout
        VkCommandBuffer copyCmd = _renderer->commandPool()->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

        // The sub resource range describes the regions of the image we will be transition
        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.layerCount = 1;

        // Transition the texture image layout to shader read, so it can be sampled from
        VkImageMemoryBarrier imageMemoryBarrier = vks::initializers::imageMemoryBarrier();
        imageMemoryBarrier.image = _image;
        imageMemoryBarrier.subresourceRange = subresourceRange;
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
        // Source pipeline stage is host write/read exection (VK_PIPELINE_STAGE_HOST_BIT)
        // Destination pipeline stage fragment shader access (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
        vkCmdPipelineBarrier(
                    copyCmd,
                    VK_PIPELINE_STAGE_HOST_BIT,
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &imageMemoryBarrier);

        _renderer->commandPool()->flushCommandBuffer(copyCmd, true);
    }

    // Create a texture sampler
    // In Vulkan textures are accessed by samplers
    // This separates all the sampling information from the texture data. This means you could have multiple sampler objects for the same texture with different settings
    // Note: Similar to the samplers available with OpenGL 3.3
    VkSamplerCreateInfo sampler = vks::initializers::samplerCreateInfo();
    sampler.magFilter = VK_FILTER_LINEAR;
    sampler.minFilter = VK_FILTER_LINEAR;
    sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler.mipLodBias = 0.0f;
    sampler.compareOp = VK_COMPARE_OP_NEVER;
    sampler.minLod = 0.0f;
    // Set max level-of-detail to mip level count of the texture
    sampler.maxLod = (useStaging) ? (float)_mip_levels : 0.0f;
    // Enable anisotropic filtering
    // This feature is optional, so we must check if it's supported on the device
    if (_renderer->device()->features().samplerAnisotropy) {
        // Use max. level of anisotropy for this example
        sampler.maxAnisotropy = _renderer->device()->properties().limits.maxSamplerAnisotropy;
        sampler.anisotropyEnable = VK_TRUE;
    } else {
        // The device does not support anisotropic filtering
        sampler.maxAnisotropy = 1.0;
        sampler.anisotropyEnable = VK_FALSE;
    }
    sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    VKUtil::checkResult(vkCreateSampler(logicalDevice, &sampler, nullptr, &_descriptor.sampler));

    // Create image view
    // Textures are not directly accessed by the shaders and
    // are abstracted by image views containing additional
    // information and sub resource ranges
    VkImageViewCreateInfo view = vks::initializers::imageViewCreateInfo();
    view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view.format = format;
    view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    // The subresource range describes the set of mip levels (and array layers) that can be accessed through this image view
    // It's possible to create multiple image views for a single image referring to different (and/or overlapping) ranges of the image
    view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view.subresourceRange.baseMipLevel = 0;
    view.subresourceRange.baseArrayLayer = 0;
    view.subresourceRange.layerCount = 1;
    // Linear tiling usually won't support mip maps
    // Only set mip map count if optimal tiling is used
    view.subresourceRange.levelCount = (useStaging) ? _mip_levels : 1;
    // The view will be based on the texture's image
    view.image = _image;
    VKUtil::checkResult(vkCreateImageView(logicalDevice, &view, nullptr, &_descriptor.imageView));
}

}
}

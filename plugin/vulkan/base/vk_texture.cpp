#include "vulkan/base/vk_texture.h"

#include "core/base/string.h"

#include "graphics/base/bitmap.h"

#include "renderer/base/render_controller.h"
#include "renderer/base/recycler.h"
#include "renderer/base/texture.h"
#include "renderer/util/render_util.h"

#include "vulkan/base/vk_device.h"
#include "vulkan/base/vk_command_pool.h"
#include "vulkan/base/vk_renderer.h"
#include "vulkan/util/vk_util.h"

namespace ark::plugin::vulkan {

namespace {

void copyBitmap(uint8_t* buf, const Bitmap& bitmap, const bytearray& imagedata, size_t imageDataSize)
{
    if(imagedata == nullptr)
        memset(buf, 0, imageDataSize);
    else if(imageDataSize == imagedata->size())
        memcpy(buf, imagedata->buf(), imageDataSize);
    else
    {
        const uint32_t steps = bitmap.width() * bitmap.height();
        const uint32_t pixelBytes = bitmap.rowBytes() / bitmap.width();
        const size_t newPixelBytes = imageDataSize / steps;
        const size_t padding = newPixelBytes - pixelBytes;
        const uint8_t* it1 = imagedata->buf();
        uint8_t* it2 = buf;
        for(uint32_t i = 0; i < steps; ++i)
        {
            memcpy(it2, it1, pixelBytes);
            memset(it2 + pixelBytes, 0xff, padding);
            it1 += pixelBytes;
            it2 += newPixelBytes;
        }
    }
}

VkImageUsageFlags toTextureUsage(const Texture::Usage usage)
{
    VkImageUsageFlags vkFlags = usage == Texture::USAGE_AUTO ? VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT : 0;
    if(usage.contains(Texture::USAGE_COLOR_ATTACHMENT))
        vkFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if(usage.contains(Texture::USAGE_DEPTH_STENCIL_ATTACHMENT))
        vkFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    if(usage.contains(Texture::USAGE_SAMPLER))
        vkFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if(usage.contains(Texture::USAGE_STORAGE))
        vkFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
    return vkFlags;
}

}

VKTexture::VKTexture(sp<Recycler> recycler, sp<VKRenderer> renderer, uint32_t width, uint32_t height, sp<Texture::Parameters> parameters)
    : Delegate(parameters->_type), _recycler(std::move(recycler)), _renderer(std::move(renderer)), _width(width), _height(height), _parameters(std::move(parameters)),
      _num_faces(_parameters->_type == Texture::TYPE_2D ? 1 : 6), _image(VK_NULL_HANDLE), _memory(VK_NULL_HANDLE), _descriptor{}
{
}

VKTexture::~VKTexture()
{
    if(_image)
        _recycler->recycle(doRecycle());
}

uint64_t VKTexture::id()
{
    return (uint64_t)(_image);
}

void VKTexture::upload(GraphicsContext& graphicsContext, const sp<Texture::Uploader>& uploader)
{
    if(uploader)
    {
        if(_image)
            uploader->update(graphicsContext, *this);
        else
            uploader->initialize(graphicsContext, *this);
    }
    else
    {
        const Texture::Format format = _parameters->_format;
        const Vector<sp<ByteArray>> imagedata(_num_faces);
        if(format == Texture::FORMAT_AUTO)
            uploadBitmap(graphicsContext, Bitmap(_width, _height, _width * 4, 4, false), imagedata);
        else
        {
            const uint8_t channels = format & Texture::FORMAT_RGBA;
            const uint32_t componentSize = RenderUtil::getComponentSize(format);
            uploadBitmap(graphicsContext, Bitmap(_width, _height, _width * channels * componentSize, channels, false), imagedata);
        }
    }
}

ResourceRecycleFunc VKTexture::recycle()
{
    return doRecycle();
}

void VKTexture::clear(GraphicsContext& /*graphicsContext*/)
{
    constexpr VkClearColorValue clearColorValue = {{0, 0, 0, 0}};
    constexpr VkImageSubresourceRange subRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, 1};
    const VkCommandBuffer clearCmdBuf = _renderer->commandPool()->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    vkCmdClearColorImage(clearCmdBuf, _image, _descriptor.imageLayout, &clearColorValue, 1, &subRange);
    _renderer->commandPool()->flushCommandBuffer(clearCmdBuf, true);
}

bool VKTexture::download(GraphicsContext& graphicsContext, Bitmap& bitmap)
{
    return false;
}

VkSamplerAddressMode toSamplerAddressMode(const Texture::Filter filter)
{
    constexpr std::array<VkSamplerAddressMode, Texture::FILTER_COUNT> vkAddressModes = {VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE};
    return vkAddressModes[filter];
}

void VKTexture::doCreateSamplerDescriptor(const VkDevice logicalDevice)
{
    // Create a texture sampler
    // In Vulkan textures are accessed by samplers
    // This separates all the sampling information from the texture data. This means you could have multiple sampler objects for the same texture with different settings
    // Note: Similar to the samplers available with OpenGL 3.3
    VkSamplerCreateInfo sampler = vks::initializers::samplerCreateInfo();
    sampler.magFilter = _parameters->_mag_filter == Texture::FILTER_NEAREST ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
    sampler.minFilter = _parameters->_min_filter == Texture::FILTER_NEAREST ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
    sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler.addressModeU = toSamplerAddressMode(_parameters->_wrap_r);
    sampler.addressModeV = toSamplerAddressMode(_parameters->_wrap_s);
    sampler.addressModeW = toSamplerAddressMode(_parameters->_wrap_t);
    sampler.mipLodBias = 0.0f;
    sampler.compareOp = VK_COMPARE_OP_NEVER;
    sampler.minLod = 0.0f;
    // Set max level-of-detail to mip level count of the texture
    sampler.maxLod = static_cast<float>(_mip_levels);
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
}

void VKTexture::uploadBitmap(GraphicsContext& /*graphicContext*/, const Bitmap& bitmap, const Vector<sp<ByteArray>>& images)
{
    DASSERT(images.size() == _num_faces);
    const bytearray& imagedata = images.at(0);
    VkFormat format = VKUtil::toTextureFormat(bitmap, _parameters->_format);

    DCHECK(_width == bitmap.width() && _height == bitmap.height(), "Uploading bitmap has different size(%d, %d) compared to Texture's(%d, %d)", bitmap.width(), bitmap.height(), _width, _height);
    _mip_levels = 1;

    _observer.notify();

    const bool isCubemap = _num_faces == 6;

    VkDevice logicalDevice = _renderer->vkLogicalDevice();
    if(_parameters->_usage.contains(Texture::USAGE_DEPTH_STENCIL_ATTACHMENT))
    {
        const VkFormat fbDepthFormat = VKUtil::getSupportedDepthFormat(_renderer->vkPhysicalDevice(), _parameters->_format, _parameters->_usage);
        VkImageCreateInfo image = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        image.format = fbDepthFormat;
        image.usage = toTextureUsage(_parameters->_usage);
        image.imageType = VK_IMAGE_TYPE_2D;
        image.extent = {_width, _height , 1};
        image.mipLevels = 1;
        image.arrayLayers = _num_faces;
        image.samples = VK_SAMPLE_COUNT_1_BIT;
        image.tiling = VK_IMAGE_TILING_OPTIMAL;
        VKUtil::createImage(_renderer->device(), image, &_image, &_memory);

        VkImageViewCreateInfo depthStencilView = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
        depthStencilView.format = fbDepthFormat;
        depthStencilView.flags = 0;
        depthStencilView.subresourceRange = {};
        depthStencilView.subresourceRange.aspectMask = VKUtil::toTextureAspect(_parameters->_usage);
        depthStencilView.subresourceRange.baseMipLevel = 0;
        depthStencilView.subresourceRange.levelCount = 1;
        depthStencilView.subresourceRange.baseArrayLayer = 0;
        depthStencilView.subresourceRange.layerCount = _num_faces;
        depthStencilView.image = _image;
        VKUtil::checkResult(vkCreateImageView(logicalDevice, &depthStencilView, nullptr, &_descriptor.imageView));

        _descriptor.imageLayout = VKUtil::toImageLayout(_parameters->_usage);
        if(_parameters->_usage.contains(Texture::USAGE_SAMPLER))
            doCreateSamplerDescriptor(logicalDevice);

        return;
    }

    {
        // Create optimal tiled target image on the device
        VkImageCreateInfo imageCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = format;
        imageCreateInfo.mipLevels = _mip_levels;
        imageCreateInfo.arrayLayers = _num_faces;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        // Set initial layout of the image to undefined
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.extent = { _width, _height, 1 };
        imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | toTextureUsage(_parameters->_usage);
        imageCreateInfo.flags = isCubemap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;

        size_t imageDataSize = imagedata ? imagedata->size() : bitmap.rowBytes() * bitmap.height();
        VkImageFormatProperties ifp = {};
        VkResult r = vkGetPhysicalDeviceImageFormatProperties(_renderer->device()->vkPhysicalDevice(), imageCreateInfo.format, imageCreateInfo.imageType, imageCreateInfo.tiling,
                                                              imageCreateInfo.usage, imageCreateInfo.flags, &ifp);
        if(r == VK_ERROR_FORMAT_NOT_SUPPORTED)
        {
            if(bitmap.channels() != 4)
            {
                DCHECK_WARN(bitmap.channels() != 4, "Image format is not supported by Vulkan, channesl: %d, converting to RGBA format", bitmap.channels());
                uint32_t rowBytes = bitmap.rowBytes() / bitmap.channels() * 4;
                imageDataSize = rowBytes * bitmap.height();
                format = imageCreateInfo.format = VKUtil::toTextureFormat(rowBytes / bitmap.width() / 4, 4, _parameters->_format);
            }
            else
                WARN("Format not supported");
        }
        VKUtil::createImage(_renderer->device(), imageCreateInfo, &_image, &_memory);

        _descriptor.imageLayout = VKUtil::toImageLayout(_parameters->_usage);
        doUploadBitmap(bitmap, imageDataSize, images);
    }

    if(_parameters->_usage == Texture::USAGE_AUTO || _parameters->_usage.contains(Texture::USAGE_SAMPLER))
        doCreateSamplerDescriptor(logicalDevice);

    // Create image view
    // Textures are not directly accessed by the shaders and
    // are abstracted by image views containing additional
    // information and sub resource ranges
    VkImageViewCreateInfo view = vks::initializers::imageViewCreateInfo();
    view.viewType = isCubemap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
    view.format = format;
    view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    // The subresource range describes the set of mip levels (and array layers) that can be accessed through this image view
    // It's possible to create multiple image views for a single image referring to different (and/or overlapping) ranges of the image
    view.subresourceRange.aspectMask = VKUtil::toTextureAspect(_parameters->_usage);
    view.subresourceRange.baseMipLevel = 0;
    view.subresourceRange.baseArrayLayer = 0;
    view.subresourceRange.layerCount = _num_faces;
    // Linear tiling usually won't support mip maps
    // Only set mip map count if optimal tiling is used
    view.subresourceRange.levelCount = _mip_levels;
    // The view will be based on the texture's image
    view.image = _image;
    VKUtil::checkResult(vkCreateImageView(logicalDevice, &view, nullptr, &_descriptor.imageView));
}

const VkDescriptorImageInfo& VKTexture::vkDescriptor() const
{
    return _descriptor;
}

Observer& VKTexture::observer()
{
    return _observer;
}

void VKTexture::doUploadBitmap(const Bitmap& bitmap, size_t imageDataSize, const Vector<bytearray>& imagedata) const
{
    VkMemoryAllocateInfo memAllocInfo = vks::initializers::memoryAllocateInfo();
    VkMemoryRequirements memReqs = {};

    VkDevice logicalDevice = _renderer->vkLogicalDevice();

    // Copy data to an optimal tiled image
    // This loads the texture data into a host local buffer that is copied to the optimal tiled image on the device

    // Create a host-visible staging buffer that contains the raw image data
    // This buffer will be the data source for copying texture data to the optimal tiled image on the device
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

    DASSERT(imageDataSize > 0);

    VkBufferCreateInfo bufferCreateInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferCreateInfo.size = imageDataSize * imagedata.size();
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
    uint8_t* data;
    VKUtil::checkResult(vkMapMemory(logicalDevice, stagingMemory, 0, memReqs.size, 0, reinterpret_cast<void**>(&data)));
    for(size_t i = 0; i < imagedata.size(); ++i)
    {
        copyBitmap(data, bitmap, imagedata.at(i), imageDataSize);
        data += imageDataSize;
    }
    vkUnmapMemory(logicalDevice, stagingMemory);

    // Setup buffer copy regions for each mip level
    uint32_t offset = 0;
    Vector<VkBufferImageCopy> bufferCopyRegions;
    for(size_t i = 0; i < imagedata.size(); ++i)
    {
        for(uint32_t j = 0; j < _mip_levels; j++)
        {
            VkBufferImageCopy bufferCopyRegion = {};
            bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            bufferCopyRegion.imageSubresource.mipLevel = j;
            bufferCopyRegion.imageSubresource.baseArrayLayer = static_cast<uint32_t>(i);
            bufferCopyRegion.imageSubresource.layerCount = 1;
            bufferCopyRegion.imageExtent.width = bitmap.width();
            bufferCopyRegion.imageExtent.height = bitmap.height();
            bufferCopyRegion.imageExtent.depth = 1;
            bufferCopyRegion.bufferOffset = offset;

            bufferCopyRegions.push_back(bufferCopyRegion);
        }
        offset += static_cast<uint32_t>(imageDataSize);
    }

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
    subresourceRange.layerCount = static_cast<uint32_t>(imagedata.size());

    // Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
    VkImageMemoryBarrier imageMemoryBarrier = vks::initializers::imageMemoryBarrier();
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
    imageMemoryBarrier.newLayout = _descriptor.imageLayout;

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

    _renderer->commandPool()->flushCommandBuffer(copyCmd, true);

    // Clean up staging resources
    vkFreeMemory(logicalDevice, stagingMemory, nullptr);
    vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
}

ResourceRecycleFunc VKTexture::doRecycle()
{
    const sp<VKDevice> device = _renderer->device();
    VkDescriptorImageInfo descriptor = _descriptor;
    VkImage image = _image;
    VkDeviceMemory memory = _memory;

    _image = VK_NULL_HANDLE;
    _memory = VK_NULL_HANDLE;
    _observer.notify();

    return [device, descriptor, image, memory](GraphicsContext&) {
        vkDestroyImageView(device->vkLogicalDevice(), descriptor.imageView, nullptr);
        vkDestroyImage(device->vkLogicalDevice(), image, nullptr);
        vkDestroySampler(device->vkLogicalDevice(), descriptor.sampler, nullptr);
        vkFreeMemory(device->vkLogicalDevice(), memory, nullptr);
    };
}

}

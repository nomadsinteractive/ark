#include "renderer/vulkan/base/vulkan_api.h"

#include <array>

#include <glm/gtc/matrix_transform.hpp>

#include "core/base/plugin_manager.h"
#include "core/inf/array.h"
#include "core/inf/dictionary.h"
#include "core/types/global.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"

#include "renderer/base/gl_context.h"
#include "renderer/base/gl_resource_manager.h"

#include "renderer/vulkan/base/buffer.h"
#include "renderer/vulkan/base/command_pool.h"
#include "renderer/vulkan/base/device.h"
#include "renderer/vulkan/base/instance.h"
#include "renderer/vulkan/base/pipeline.h"
#include "renderer/vulkan/base/pipeline_factory.h"
#include "renderer/vulkan/base/render_target.h"
#include "renderer/vulkan/base/texture.h"
#include "renderer/vulkan/util/vulkan_tools.h"
#include "renderer/vulkan/util/vulkan_debug.h"

#include "generated/vulkan_plugin.h"

#define VERTEX_BUFFER_BIND_ID 0
#define _VULKAN_VALIDATE true

namespace ark {

#ifdef _WIN32
    extern HINSTANCE gInstance;
    extern HWND gWnd;
#endif

namespace vulkan {

VulkanAPI::VulkanAPI(const sp<GLResourceManager>& resourceManager)
    : _resource_manager(resourceManager)
{
}

VulkanAPI::~VulkanAPI()
{
    _device->waitIdle();


    vertexBuffer.destroy();
    indexBuffer.destroy();

    _render_target->commandPool()->destroyCommandBuffers(_command_buffers.size(), _command_buffers.data());

    for (auto& shaderModule : shaderModules)
    {
        vkDestroyShaderModule(_logical_device, shaderModule, nullptr);
    }

#if defined(_DIRECT2DISPLAY)

#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    wl_shell_surface_destroy(shell_surface);
    wl_surface_destroy(surface);
    if (keyboard)
        wl_keyboard_destroy(keyboard);
    if (pointer)
        wl_pointer_destroy(pointer);
    wl_seat_destroy(seat);
    wl_shell_destroy(shell);
    wl_compositor_destroy(compositor);
    wl_registry_destroy(registry);
    wl_display_disconnect(display);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    // todo : android cleanup (if required)
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    xcb_destroy_window(connection, window);
    xcb_disconnect(connection);
#endif
}

void VulkanAPI::initialize(GLContext& /*glContext*/)
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);
    zoom = -2.5f;
    _instance = sp<Instance>::make();
    _instance->initialize();
    _device = sp<Device>::make(_instance, _instance->physicalDevices()[0]);

    _queue = _device->queue();
    _logical_device = _device->logicalDevice();

    _render_target = sp<RenderTarget>::make(_device);
    _uniforms = sp<Buffer>::make(_device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(uboVS));

    updateUniformBuffers();
    generateQuad();

    _pipeline_factory = sp<PipelineFactory>::make(_resource_manager, _render_target);
    _pipeline_factory->_texture = sp<Texture>::make(_resource_manager, _render_target->commandPool());
    _pipeline_factory->_buffer = _uniforms;
    _pipeline = _pipeline_factory->build();

    buildCommandBuffers();
    prepared = true;
}

void VulkanAPI::render()
{
    if (!prepared)
        return;
    draw();
}

void VulkanAPI::updateUniformBuffers()
{
    // Vertex shader
    uboVS.projection = glm::perspective(glm::radians(60.0f), (float)_render_target->width() / (float)_render_target->height(), 0.001f, 256.0f);
    glm::mat4 viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, zoom));

    uboVS.model = viewMatrix * glm::translate(glm::mat4(1.0f), cameraPos);
    uboVS.model = glm::rotate(uboVS.model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    uboVS.model = glm::rotate(uboVS.model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    uboVS.model = glm::rotate(uboVS.model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    uboVS.viewPos = glm::vec4(0.0f, 0.0f, -zoom, 0.0f);

    _uniforms->upload(&uboVS, sizeof(uboVS));
}

void VulkanAPI::buildCommandBuffers()
{
    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

    VkClearValue clearValues[2];
    clearValues[0].color = defaultClearColor;
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
    renderPassBeginInfo.renderPass = _render_target->renderPass();
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = _render_target->width();
    renderPassBeginInfo.renderArea.extent.height = _render_target->height();
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;

    _command_buffers = _render_target->makeCommandBuffers();
    for (size_t i = 0; i < _command_buffers.size(); ++i)
    {
        // Set target frame buffer
        renderPassBeginInfo.framebuffer = _render_target->frameBuffers()[i];

        checkResult(vkBeginCommandBuffer(_command_buffers[i], &cmdBufInfo));

        vkCmdBeginRenderPass(_command_buffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = vks::initializers::viewport((float)_render_target->width(), (float)_render_target->height(), 0.0f, 1.0f);
        vkCmdSetViewport(_command_buffers[i], 0, 1, &viewport);

        VkRect2D scissor = vks::initializers::rect2D(_render_target->width(), _render_target->height(), 0, 0);
        vkCmdSetScissor(_command_buffers[i], 0, 1, &scissor);

        vkCmdBindDescriptorSets(_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline->layout(), 0, 1, &_pipeline->descriptorSet(), 0, nullptr);
        vkCmdBindPipeline(_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline->pipeline());

        VkDeviceSize offsets[1] = { 0 };
        vkCmdBindVertexBuffers(_command_buffers[i], VERTEX_BUFFER_BIND_ID, 1, &vertexBuffer.buffer, offsets);
        vkCmdBindIndexBuffer(_command_buffers[i], indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(_command_buffers[i], indexCount, 1, 0, 0, 0);

        vkCmdEndRenderPass(_command_buffers[i]);

        checkResult(vkEndCommandBuffer(_command_buffers[i]));
    }
}

void VulkanAPI::generateQuad()
{
    // Setup vertices for a single uv-mapped quad made from two triangles
    std::vector<Vertex> vertices =
    {
        { {  1.0f,  1.0f, 0.0f }, { 1.0f, 1.0f },{ 0.0f, 0.0f, 1.0f } },
        { { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f } },
        { { -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } },
        { {  1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } }
    };

    // Setup indices
    std::vector<uint32_t> indices = { 0,1,2, 2,3,0 };
    indexCount = static_cast<uint32_t>(indices.size());

    // Create buffers
    // For the sake of simplicity we won't stage the vertex data to the gpu memory
    // Vertex buffer
    checkResult(_device->createBuffer(
                        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        &vertexBuffer,
                        vertices.size() * sizeof(Vertex),
                        vertices.data()));
    // Index buffer
    checkResult(_device->createBuffer(
                        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        &indexBuffer,
                        indices.size() * sizeof(uint32_t),
                        indices.data()));
}

void VulkanAPI::draw()
{
    uint32_t currentBuffer = _render_target->acquire();
    _render_target->submit(&_command_buffers[currentBuffer]);
    _render_target->swap(currentBuffer);
}

void VulkanAPI::checkResult(VkResult result)
{
    DCHECK(result == VK_SUCCESS, "Vulkan error: %s", vks::tools::errorString(result).c_str());
}

//void VulkanAPI::flushCommandBuffer(VkCommandBuffer commandBuffer) const
//{
//    DASSERT(commandBuffer != VK_NULL_HANDLE);

//    checkResult(vkEndCommandBuffer(commandBuffer));

//    VkSubmitInfo submitInfo = {};
//    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//    submitInfo.commandBufferCount = 1;
//    submitInfo.pCommandBuffers = &commandBuffer;

//    // Create fence to ensure that the command buffer has finished executing
//    VkFenceCreateInfo fenceCreateInfo = {};
//    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
//    fenceCreateInfo.flags = 0;
//    VkFence fence;
//    checkResult(vkCreateFence(_logical_device, &fenceCreateInfo, nullptr, &fence));

//    // Submit to the queue
//    checkResult(vkQueueSubmit(_queue, 1, &submitInfo, fence));
//    // Wait for the fence to signal that command buffer has finished executing
//    checkResult(vkWaitForFences(_logical_device, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));

//    vkDestroyFence(_logical_device, fence, nullptr);
//    vkFreeCommandBuffers(_logical_device, _render_target->commandPool(), 1, &commandBuffer);
//}

VkPipelineShaderStageCreateInfo VulkanAPI::loadShader(VkDevice device, std::string fileName, VkShaderStageFlagBits stage)
{
    VkPipelineShaderStageCreateInfo shaderStage = {};
    shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.stage = stage;
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    shaderStage.module = vks::tools::loadShader(androidApp->activity->assetManager, fileName.c_str(), device);
#else
    shaderStage.module = vks::tools::loadShader(fileName.c_str(), device);
#endif
    shaderStage.pName = "main"; // todo : make param
    DASSERT(shaderStage.module != VK_NULL_HANDLE);
//    shaderModules.push_back(shaderStage.module);
    return shaderStage;
}

//void VulkanAPI::destroyCommandBuffers() const
//{
//    vkFreeCommandBuffers(_logical_device, _render_target->commandPool(), static_cast<uint32_t>(_command_buffers.size()), _command_buffers.data());
//}

//void VulkanAPI::loadTexture()
//{
//    const std::string filename = "texture.png";
//    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

//    const bitmap tex = _resource_manager->bitmapLoader()->get(filename);

//    texture.width = tex->width();
//    texture.height = tex->height();
//    texture.mipLevels = 1;

//    // We prefer using staging to copy the texture data to a device local optimal image
//    VkBool32 useStaging = true;

//    // Only use linear tiling if forced
//    bool forceLinearTiling = false;
//    if (forceLinearTiling) {
//        // Don't use linear if format is not supported for (linear) shader sampling
//        // Get device properites for the requested texture format
//        VkFormatProperties formatProperties;
//        vkGetPhysicalDeviceFormatProperties(_device->physicalDevice(), format, &formatProperties);
//        useStaging = !(formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
//    }

//    VkMemoryAllocateInfo memAllocInfo = vks::initializers::memoryAllocateInfo();
//    VkMemoryRequirements memReqs = {};

//    if (useStaging) {
//        // Copy data to an optimal tiled image
//        // This loads the texture data into a host local buffer that is copied to the optimal tiled image on the device

//        // Create a host-visible staging buffer that contains the raw image data
//        // This buffer will be the data source for copying texture data to the optimal tiled image on the device
//        VkBuffer stagingBuffer;
//        VkDeviceMemory stagingMemory;

//        VkBufferCreateInfo bufferCreateInfo = vks::initializers::bufferCreateInfo();
//        bufferCreateInfo.size = tex->bytes()->size();
//        // This buffer is used as a transfer source for the buffer copy
//        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
//        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//        checkResult(vkCreateBuffer(_logical_device, &bufferCreateInfo, nullptr, &stagingBuffer));

//        // Get memory requirements for the staging buffer (alignment, memory type bits)
//        vkGetBufferMemoryRequirements(_logical_device, stagingBuffer, &memReqs);
//        memAllocInfo.allocationSize = memReqs.size;
//        // Get memory type index for a host visible buffer
//        memAllocInfo.memoryTypeIndex = _device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
//        checkResult(vkAllocateMemory(_logical_device, &memAllocInfo, nullptr, &stagingMemory));
//        checkResult(vkBindBufferMemory(_logical_device, stagingBuffer, stagingMemory, 0));

//        // Copy texture data into host local staging buffer
//        uint8_t *data;
//        checkResult(vkMapMemory(_logical_device, stagingMemory, 0, memReqs.size, 0, (void **)&data));
//        memcpy(data, tex->bytes()->buf(), tex->bytes()->size());
//        vkUnmapMemory(_logical_device, stagingMemory);

//        // Setup buffer copy regions for each mip level
//        std::vector<VkBufferImageCopy> bufferCopyRegions;
//        uint32_t offset = 0;

//        for (uint32_t i = 0; i < texture.mipLevels; i++) {
//            VkBufferImageCopy bufferCopyRegion = {};
//            bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//            bufferCopyRegion.imageSubresource.mipLevel = i;
//            bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
//            bufferCopyRegion.imageSubresource.layerCount = 1;
//            bufferCopyRegion.imageExtent.width = tex->width();
//            bufferCopyRegion.imageExtent.height = tex->height();
//            bufferCopyRegion.imageExtent.depth = 1;
//            bufferCopyRegion.bufferOffset = offset;

//            bufferCopyRegions.push_back(bufferCopyRegion);

//            offset += static_cast<uint32_t>(tex->bytes()->size());
//        }

//        // Create optimal tiled target image on the device
//        VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
//        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
//        imageCreateInfo.format = format;
//        imageCreateInfo.mipLevels = texture.mipLevels;
//        imageCreateInfo.arrayLayers = 1;
//        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
//        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
//        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//        // Set initial layout of the image to undefined
//        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//        imageCreateInfo.extent = { texture.width, texture.height, 1 };
//        imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
//        checkResult(vkCreateImage(_logical_device, &imageCreateInfo, nullptr, &texture.image));

//        vkGetImageMemoryRequirements(_logical_device, texture.image, &memReqs);
//        memAllocInfo.allocationSize = memReqs.size;
//        memAllocInfo.memoryTypeIndex = _device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
//        checkResult(vkAllocateMemory(_logical_device, &memAllocInfo, nullptr, &texture.deviceMemory));
//        checkResult(vkBindImageMemory(_logical_device, texture.image, texture.deviceMemory, 0));

//        VkCommandBuffer copyCmd = _render_target->commandPool()->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

//        // Image memory barriers for the texture image

//        // The sub resource range describes the regions of the image that will be transitioned using the memory barriers below
//        VkImageSubresourceRange subresourceRange = {};
//        // Image only contains color data
//        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//        // Start at first mip level
//        subresourceRange.baseMipLevel = 0;
//        // We will transition on all mip levels
//        subresourceRange.levelCount = texture.mipLevels;
//        // The 2D texture only has one layer
//        subresourceRange.layerCount = 1;

//        // Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
//        VkImageMemoryBarrier imageMemoryBarrier = vks::initializers::imageMemoryBarrier();;
//        imageMemoryBarrier.image = texture.image;
//        imageMemoryBarrier.subresourceRange = subresourceRange;
//        imageMemoryBarrier.srcAccessMask = 0;
//        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

//        // Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
//        // Source pipeline stage is host write/read exection (VK_PIPELINE_STAGE_HOST_BIT)
//        // Destination pipeline stage is copy command exection (VK_PIPELINE_STAGE_TRANSFER_BIT)
//        vkCmdPipelineBarrier(
//                    copyCmd,
//                    VK_PIPELINE_STAGE_HOST_BIT,
//                    VK_PIPELINE_STAGE_TRANSFER_BIT,
//                    0,
//                    0, nullptr,
//                    0, nullptr,
//                    1, &imageMemoryBarrier);

//        // Copy mip levels from staging buffer
//        vkCmdCopyBufferToImage(
//                    copyCmd,
//                    stagingBuffer,
//                    texture.image,
//                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
//                    static_cast<uint32_t>(bufferCopyRegions.size()),
//                    bufferCopyRegions.data());

//        // Once the data has been uploaded we transfer to the texture image to the shader read layout, so it can be sampled from
//        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
//        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

//        // Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
//        // Source pipeline stage stage is copy command exection (VK_PIPELINE_STAGE_TRANSFER_BIT)
//        // Destination pipeline stage fragment shader access (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
//        vkCmdPipelineBarrier(
//                    copyCmd,
//                    VK_PIPELINE_STAGE_TRANSFER_BIT,
//                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
//                    0,
//                    0, nullptr,
//                    0, nullptr,
//                    1, &imageMemoryBarrier);

//        // Store current layout for later reuse
//        texture.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

//        _render_target->commandPool()->flushCommandBuffer(copyCmd, true);

//        // Clean up staging resources
//        vkFreeMemory(_logical_device, stagingMemory, nullptr);
//        vkDestroyBuffer(_logical_device, stagingBuffer, nullptr);
//    } else {
//        // Copy data to a linear tiled image

//        VkImage mappableImage;
//        VkDeviceMemory mappableMemory;

//        // Load mip map level 0 to linear tiling image
//        VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
//        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
//        imageCreateInfo.format = format;
//        imageCreateInfo.mipLevels = 1;
//        imageCreateInfo.arrayLayers = 1;
//        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
//        imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
//        imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
//        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
//        imageCreateInfo.extent = { texture.width, texture.height, 1 };
//        checkResult(vkCreateImage(_logical_device, &imageCreateInfo, nullptr, &mappableImage));

//        // Get memory requirements for this image like size and alignment
//        vkGetImageMemoryRequirements(_logical_device, mappableImage, &memReqs);
//        // Set memory allocation size to required memory size
//        memAllocInfo.allocationSize = memReqs.size;
//        // Get memory type that can be mapped to host memory
//        memAllocInfo.memoryTypeIndex = _device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
//        checkResult(vkAllocateMemory(_logical_device, &memAllocInfo, nullptr, &mappableMemory));
//        checkResult(vkBindImageMemory(_logical_device, mappableImage, mappableMemory, 0));

//        // Map image memory
//        void *data;
//        checkResult(vkMapMemory(_logical_device, mappableMemory, 0, memReqs.size, 0, &data));
//        // Copy image data of the first mip level into memory
//        memcpy(data, tex->bytes()->buf(), tex->bytes()->size());
//        vkUnmapMemory(_logical_device, mappableMemory);

//        // Linear tiled images don't need to be staged and can be directly used as textures
//        texture.image = mappableImage;
//        texture.deviceMemory = mappableMemory;
//        texture.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

//        // Setup image memory barrier transfer image to shader read layout
//        VkCommandBuffer copyCmd = _render_target->commandPool()->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

//        // The sub resource range describes the regions of the image we will be transition
//        VkImageSubresourceRange subresourceRange = {};
//        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//        subresourceRange.baseMipLevel = 0;
//        subresourceRange.levelCount = 1;
//        subresourceRange.layerCount = 1;

//        // Transition the texture image layout to shader read, so it can be sampled from
//        VkImageMemoryBarrier imageMemoryBarrier = vks::initializers::imageMemoryBarrier();;
//        imageMemoryBarrier.image = texture.image;
//        imageMemoryBarrier.subresourceRange = subresourceRange;
//        imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
//        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
//        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
//        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

//        // Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
//        // Source pipeline stage is host write/read exection (VK_PIPELINE_STAGE_HOST_BIT)
//        // Destination pipeline stage fragment shader access (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
//        vkCmdPipelineBarrier(
//                    copyCmd,
//                    VK_PIPELINE_STAGE_HOST_BIT,
//                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
//                    0,
//                    0, nullptr,
//                    0, nullptr,
//                    1, &imageMemoryBarrier);

//        _render_target->commandPool()->flushCommandBuffer(copyCmd, true);
//    }

//    // Create a texture sampler
//    // In Vulkan textures are accessed by samplers
//    // This separates all the sampling information from the texture data. This means you could have multiple sampler objects for the same texture with different settings
//    // Note: Similar to the samplers available with OpenGL 3.3
//    VkSamplerCreateInfo sampler = vks::initializers::samplerCreateInfo();
//    sampler.magFilter = VK_FILTER_LINEAR;
//    sampler.minFilter = VK_FILTER_LINEAR;
//    sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
//    sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//    sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//    sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//    sampler.mipLodBias = 0.0f;
//    sampler.compareOp = VK_COMPARE_OP_NEVER;
//    sampler.minLod = 0.0f;
//    // Set max level-of-detail to mip level count of the texture
//    sampler.maxLod = (useStaging) ? (float)texture.mipLevels : 0.0f;
//    // Enable anisotropic filtering
//    // This feature is optional, so we must check if it's supported on the device
//    if (_device->features().samplerAnisotropy) {
//        // Use max. level of anisotropy for this example
//        sampler.maxAnisotropy = _device->properties().limits.maxSamplerAnisotropy;
//        sampler.anisotropyEnable = VK_TRUE;
//    } else {
//        // The device does not support anisotropic filtering
//        sampler.maxAnisotropy = 1.0;
//        sampler.anisotropyEnable = VK_FALSE;
//    }
//    sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
//    checkResult(vkCreateSampler(_logical_device, &sampler, nullptr, &texture.sampler));

//    // Create image view
//    // Textures are not directly accessed by the shaders and
//    // are abstracted by image views containing additional
//    // information and sub resource ranges
//    VkImageViewCreateInfo view = vks::initializers::imageViewCreateInfo();
//    view.viewType = VK_IMAGE_VIEW_TYPE_2D;
//    view.format = format;
//    view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
//    // The subresource range describes the set of mip levels (and array layers) that can be accessed through this image view
//    // It's possible to create multiple image views for a single image referring to different (and/or overlapping) ranges of the image
//    view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//    view.subresourceRange.baseMipLevel = 0;
//    view.subresourceRange.baseArrayLayer = 0;
//    view.subresourceRange.layerCount = 1;
//    // Linear tiling usually won't support mip maps
//    // Only set mip map count if optimal tiling is used
//    view.subresourceRange.levelCount = (useStaging) ? texture.mipLevels : 1;
//    // The view will be based on the texture's image
//    view.image = texture.image;
//    checkResult(vkCreateImageView(_logical_device, &view, nullptr, &texture.view));
//}


//void VulkanAPI::flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free) const
//{
//    if (commandBuffer == VK_NULL_HANDLE)
//    {
//        return;
//    }

//    checkResult(vkEndCommandBuffer(commandBuffer));

//    VkSubmitInfo submitInfo = {};
//    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//    submitInfo.commandBufferCount = 1;
//    submitInfo.pCommandBuffers = &commandBuffer;

//    checkResult(vkQueueSubmit(_queue, 1, &submitInfo, VK_NULL_HANDLE));
//    checkResult(vkQueueWaitIdle(_queue));

//    if (free)
//    {
//        vkFreeCommandBuffers(_logical_device, _render_target->commandPool(), 1, &commandBuffer);
//    }
//}

//void VulkanAPI::destroyTextureImage(VulkanAPI::Texture texture) const
//{
//    vkDestroyImageView(_logical_device, texture.view, nullptr);
//    vkDestroyImage(_logical_device, texture.image, nullptr);
//    vkDestroySampler(_logical_device, texture.sampler, nullptr);
//    vkFreeMemory(_logical_device, texture.deviceMemory, nullptr);
//}

}
}

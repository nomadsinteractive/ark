#include "renderer/vulkan/base/vk_framebuffer.h"

#include "graphics/inf/render_command.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/recycler.h"
#include "renderer/base/texture.h"

#include "renderer/vulkan/base/vk_command_pool.h"
#include "renderer/vulkan/base/vk_device.h"
#include "renderer/vulkan/base/vk_graphics_context.h"
#include "renderer/vulkan/base/vk_renderer.h"
#include "renderer/vulkan/base/vk_texture.h"
#include "renderer/vulkan/util/vulkan_initializers.hpp"
#include "renderer/vulkan/util/vulkan_tools.h"
#include "renderer/vulkan/util/vk_util.h"

namespace ark {
namespace vulkan {

VKFramebuffer::VKFramebuffer(const sp<VKRenderer>& renderer, const sp<Recycler>& recycler, std::vector<sp<Texture>> colorAttachments, sp<Texture> depthStencilAttachments)
    : _renderer(renderer), _recycler(recycler), _texture(colorAttachments.at(0)), _color_attachments(std::move(colorAttachments)), _depth_stencil_attachment(std::move(depthStencilAttachments)),
      _depthstencil_image(VK_NULL_HANDLE), _depthstencil_memory(VK_NULL_HANDLE), _depthstencil_view(VK_NULL_HANDLE), _command_buffer(VK_NULL_HANDLE),
      _command_buffer_begin_info(vks::initializers::commandBufferBeginInfo()), _render_pass_begin_info(vks::initializers::renderPassBeginInfo()), _scissor(getFramebufferScissor()),
      _viewport(vks::initializers::viewport(static_cast<float>(_scissor.extent.width), static_cast<float>(_scissor.extent.height), 0, 1.0f))
{
    _clear_values[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
    _clear_values[1].depthStencil = { 1.0f, 0 };

    _render_pass_begin_info.renderArea = _scissor;
    _render_pass_begin_info.clearValueCount = 2;
    _render_pass_begin_info.pClearValues = _clear_values;

    _submit_info.pSignalSemaphores = &_semaphore;
    _submit_info.commandBufferCount = 1;
    _submit_info.pCommandBuffers = &_command_buffer;
}

VKFramebuffer::~VKFramebuffer()
{
    _recycler->recycle(*this);
}

uint64_t VKFramebuffer::id()
{
    return 0;
}

void VKFramebuffer::upload(GraphicsContext& graphicsContext, const sp<Uploader>& /*uploader*/)
{
    VkDevice device = _renderer->vkLogicalDevice();
    uint32_t width = _scissor.extent.width;
    uint32_t height = _scissor.extent.height;

    VkImageCreateInfo image = vks::initializers::imageCreateInfo();
    VkFormat fbDepthFormat;
    VkBool32 validDepthFormat = vks::tools::getSupportedDepthFormat(_renderer->vkPhysicalDevice(), &fbDepthFormat);
    DASSERT(validDepthFormat);

    image.format = fbDepthFormat;
    image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    image.imageType = VK_IMAGE_TYPE_2D;
    image.extent = {width, height , 1};
    image.mipLevels = 1;
    image.arrayLayers = 1;
    image.samples = VK_SAMPLE_COUNT_1_BIT;
    image.tiling = VK_IMAGE_TILING_OPTIMAL;
    VKUtil::createImage(_renderer->device(), image, &_depthstencil_image, &_depthstencil_memory);

    VkImageViewCreateInfo depthStencilView = vks::initializers::imageViewCreateInfo();
    depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depthStencilView.format = fbDepthFormat;
    depthStencilView.flags = 0;
    depthStencilView.subresourceRange = {};
    depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    depthStencilView.subresourceRange.baseMipLevel = 0;
    depthStencilView.subresourceRange.levelCount = 1;
    depthStencilView.subresourceRange.baseArrayLayer = 0;
    depthStencilView.subresourceRange.layerCount = 1;
    depthStencilView.image = _depthstencil_image;
    VKUtil::checkResult(vkCreateImageView(device, &depthStencilView, nullptr, &_depthstencil_view));

    std::array<VkAttachmentDescription, 2> attchmentDescriptions = {};
    // Color attachment
    attchmentDescriptions[0].format = VK_FORMAT_R8G8B8A8_UNORM;
    attchmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attchmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attchmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attchmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attchmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attchmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attchmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    // Depth attachment
    attchmentDescriptions[1].format = fbDepthFormat;
    attchmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attchmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attchmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attchmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attchmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attchmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attchmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkAttachmentReference depthReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorReference;
    subpassDescription.pDepthStencilAttachment = &depthReference;

    // Use subpass dependencies for layout transitions
    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    // Create the actual renderpass
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attchmentDescriptions.size());
    renderPassInfo.pAttachments = attchmentDescriptions.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDescription;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    VKUtil::checkResult(vkCreateRenderPass(device, &renderPassInfo, nullptr, &_render_pass_begin_info.renderPass));

    const sp<VKTexture> vkTexture = _texture->delegate();
    if(!vkTexture->id())
        vkTexture->upload(graphicsContext, nullptr);

    VkImageView attachments[2];
    attachments[0] = vkTexture->vkDescriptor().imageView;
    attachments[1] = _depthstencil_view;

    VkFramebufferCreateInfo fbufCreateInfo = vks::initializers::framebufferCreateInfo();
    fbufCreateInfo.renderPass = _render_pass_begin_info.renderPass;
    fbufCreateInfo.attachmentCount = 2;
    fbufCreateInfo.pAttachments = attachments;
    fbufCreateInfo.width = width;
    fbufCreateInfo.height = height;
    fbufCreateInfo.layers = 1;

    VKUtil::checkResult(vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &_render_pass_begin_info.framebuffer));

    _descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    _descriptor.imageView = vkTexture->vkDescriptor().imageView;
    _descriptor.sampler = vkTexture->vkDescriptor().sampler;

    _command_buffer = _renderer->commandPool()->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, false);

    VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
    VKUtil::checkResult(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &_semaphore));
}

Resource::RecycleFunc VKFramebuffer::recycle()
{
    const sp<VKDevice> device = _renderer->device();

    VkImage depthstencilImage = _depthstencil_image;
    VkDeviceMemory depthstencilMemory = _depthstencil_memory;
    VkImageView depthstencilView = _depthstencil_view;

    VkFramebuffer framebuffer = _render_pass_begin_info.framebuffer;
    VkRenderPass renderPass = _render_pass_begin_info.renderPass;

    _depthstencil_image = VK_NULL_HANDLE;
    _depthstencil_memory = VK_NULL_HANDLE;
    _depthstencil_view = VK_NULL_HANDLE;
    _render_pass_begin_info.framebuffer = VK_NULL_HANDLE;
    _render_pass_begin_info.renderPass = VK_NULL_HANDLE;

    return [=](GraphicsContext&) {
        VkDevice logicalDevice = device->vkLogicalDevice();
        vkDestroyImageView(logicalDevice, depthstencilView, nullptr);
        vkDestroyImage(logicalDevice, depthstencilImage, nullptr);
        vkFreeMemory(logicalDevice, depthstencilMemory, nullptr);
        vkDestroyRenderPass(logicalDevice, renderPass, nullptr);
        vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);
    };
}

const sp<Texture>& VKFramebuffer::texture() const
{
    return _texture;
}

void VKFramebuffer::beginCommandBuffer(GraphicsContext& graphicsContext)
{
    VKUtil::checkResult(vkBeginCommandBuffer(_command_buffer, &_command_buffer_begin_info));

    vkCmdBeginRenderPass(_command_buffer, &_render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdSetViewport(_command_buffer, 0, 1, &_viewport);
    vkCmdSetScissor(_command_buffer, 0, 1, &_scissor);

    graphicsContext.attachments().ensure<VKGraphicsContext>()->pushCommandBuffer(_command_buffer);

}

void VKFramebuffer::endCommandBuffer(GraphicsContext& graphicsContext)
{
    vkCmdEndRenderPass(_command_buffer);
    VKUtil::checkResult(vkEndCommandBuffer(_command_buffer));
    graphicsContext.attachments().ensure<VKGraphicsContext>()->popCommandBuffer();
}

void VKFramebuffer::submit(GraphicsContext& graphicsContext)
{
    const sp<VKGraphicsContext>& vkContext = graphicsContext.attachments().ensure<VKGraphicsContext>();
    vkContext->addSubmitInfo(1, &_command_buffer, 1, &_semaphore);
}

VkRect2D VKFramebuffer::getFramebufferScissor() const
{
    DASSERT(_color_attachments.size() > 0);
    const sp<Texture>& texture = _color_attachments.at(0);
    return vks::initializers::rect2D(texture->width(), texture->height(), 0, 0);
}

}
}

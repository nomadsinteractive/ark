#include "renderer/vulkan/base/vk_framebuffer.h"

#include "graphics/inf/render_command.h"

#include "renderer/base/framebuffer.h"
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

namespace ark::vulkan {

VKFramebuffer::VKFramebuffer(const sp<VKRenderer>& renderer, const sp<Recycler>& recycler, std::vector<sp<Texture>> colorAttachments, sp<Texture> depthStencilAttachments, int32_t clearMask)
    : _stub(sp<Stub>::make(renderer, recycler, std::move(colorAttachments), std::move(depthStencilAttachments), clearMask))
{
}

VKFramebuffer::~VKFramebuffer()
{
    _stub->_recycler->recycle(*this);
}

uint64_t VKFramebuffer::id()
{
    return (uint64_t) _stub->_render_pass_begin_info.framebuffer;
}

void VKFramebuffer::upload(GraphicsContext& /*graphicsContext*/)
{
    _stub->initialize();
}

ResourceRecycleFunc VKFramebuffer::recycle()
{
    const sp<VKDevice> device = _stub->_renderer->device();

    VkImage depthstencilImage = _stub->_depthstencil_image;
    VkDeviceMemory depthstencilMemory = _stub->_depthstencil_memory;
    VkImageView depthstencilView = _stub->_depthstencil_view;

    VkFramebuffer framebuffer = _stub->_render_pass_begin_info.framebuffer;
    VkRenderPass renderPass = _stub->_render_pass_begin_info.renderPass;

    _stub->_depthstencil_image = VK_NULL_HANDLE;
    _stub->_depthstencil_memory = VK_NULL_HANDLE;
    _stub->_depthstencil_view = VK_NULL_HANDLE;
    _stub->_render_pass_begin_info.framebuffer = VK_NULL_HANDLE;
    _stub->_render_pass_begin_info.renderPass = VK_NULL_HANDLE;

    return [=](GraphicsContext&) {
        VkDevice logicalDevice = device->vkLogicalDevice();
        if(depthstencilImage) {
            vkDestroyImageView(logicalDevice, depthstencilView, nullptr);
            vkDestroyImage(logicalDevice, depthstencilImage, nullptr);
            vkFreeMemory(logicalDevice, depthstencilMemory, nullptr);
        }
        vkDestroyRenderPass(logicalDevice, renderPass, nullptr);
        vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);
    };
}

void VKFramebuffer::beginCommandBuffer(GraphicsContext& graphicsContext)
{
    graphicsContext.attachments().ensure<VKGraphicsContext>()->pushState(_stub);
}

void VKFramebuffer::endCommandBuffer(GraphicsContext& graphicsContext)
{
    graphicsContext.attachments().ensure<VKGraphicsContext>()->popState();
}

VkRect2D VKFramebuffer::Stub::getFramebufferScissor() const
{
    DASSERT(_color_attachments.size() > 0);
    const sp<Texture>& texture = _color_attachments.at(0);
    return vks::initializers::rect2D(texture->width(), texture->height(), 0, 0);
}

VKFramebuffer::Stub::Stub(const sp<VKRenderer>& renderer, const sp<Recycler>& recycler, std::vector<sp<Texture> > colorAttachments, sp<Texture> depthStencilAttachments, int32_t clearMask)
    : _renderer(renderer), _recycler(recycler), _color_attachments(std::move(colorAttachments)), _depth_stencil_attachment(std::move(depthStencilAttachments)),
      _depthstencil_image(VK_NULL_HANDLE), _depthstencil_memory(VK_NULL_HANDLE), _depthstencil_view(VK_NULL_HANDLE), _command_buffer(VK_NULL_HANDLE),
      _render_pass_begin_info(vks::initializers::renderPassBeginInfo()), _scissor(getFramebufferScissor()),
      _viewport(vks::initializers::viewport(static_cast<float>(_scissor.extent.width), static_cast<float>(_scissor.extent.height), 0, 1.0f))
{
    VkClearValue clearColor;
    VkClearValue clearDepthStencil;
    clearColor.color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
    clearDepthStencil.depthStencil = { 1.0f, 0 };

    if(clearMask & Framebuffer::CLEAR_MASK_COLOR)
        for(uint32_t i = 0; i < _color_attachments.size(); ++i)
            _clear_values.push_back(clearColor);
    if(clearMask & Framebuffer::CLEAR_MASK_DEPTH_STENCIL)
        _clear_values.push_back(clearDepthStencil);

    _render_pass_begin_info.renderArea = _scissor;
    _render_pass_begin_info.clearValueCount = static_cast<uint32_t>(_clear_values.size());
    _render_pass_begin_info.pClearValues = _clear_values.data();
}

void VKFramebuffer::Stub::initialize()
{
    _command_buffer = _renderer->commandPool()->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, false);
}

VkCommandBuffer VKFramebuffer::Stub::vkCommandBuffer()
{
    return _command_buffer;
}

VkRenderPass VKFramebuffer::Stub::create(const PipelineDescriptor& bindings)
{
    VkDevice device = _renderer->vkLogicalDevice();
    uint32_t width = _scissor.extent.width;
    uint32_t height = _scissor.extent.height;

    VkFormat fbDepthFormat;
    VkBool32 validDepthFormat = vks::tools::getSupportedDepthFormat(_renderer->vkPhysicalDevice(), &fbDepthFormat);
    DASSERT(validDepthFormat);

    std::vector<VkAttachmentDescription> attachmentDescriptions;
    std::vector<VkAttachmentReference> attachmentReferences;
    std::vector<VkImageView> attachments;

    for(const Texture& i : _color_attachments)
    {
        VkAttachmentDescription colorAttachmentDescription = {};
        colorAttachmentDescription.format = VKUtil::toTextureFormat(i.parameters()->_format);
        colorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        attachmentDescriptions.push_back(colorAttachmentDescription);

        attachmentReferences.push_back({ static_cast<uint32_t>(attachmentReferences.size()), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

        const sp<VKTexture> vkTexture = i.delegate();
        DCHECK(vkTexture->id(), "VKTexture uninitialized");
        attachments.push_back(vkTexture->vkDescriptor().imageView);
    }

    VkImageView depthstencilView = _depth_stencil_attachment ? _depth_stencil_attachment->delegate().cast<VKTexture>()->vkDescriptor().imageView : VK_NULL_HANDLE;
    if(depthstencilView)
    {
        attachments.push_back(depthstencilView);

        VkAttachmentDescription depthAttachmentDescription = {};
        depthAttachmentDescription.format = fbDepthFormat;
        depthAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachmentDescriptions.push_back(depthAttachmentDescription);
    }
    VkAttachmentReference depthReference = { static_cast<uint32_t>(attachmentReferences.size()), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = static_cast<uint32_t>(attachmentReferences.size());
    subpassDescription.pColorAttachments = attachmentReferences.data();
    subpassDescription.pDepthStencilAttachment = depthstencilView ? &depthReference : nullptr;

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
    VkRenderPassCreateInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
    renderPassInfo.pAttachments = attachmentDescriptions.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDescription;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    VKUtil::checkResult(vkCreateRenderPass(device, &renderPassInfo, nullptr, &_render_pass_begin_info.renderPass));

    VkFramebufferCreateInfo fbufCreateInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
    fbufCreateInfo.renderPass = _render_pass_begin_info.renderPass;
    fbufCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    fbufCreateInfo.pAttachments = attachments.data();
    fbufCreateInfo.width = width;
    fbufCreateInfo.height = height;
    fbufCreateInfo.layers = 1;

    VKUtil::checkResult(vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &_render_pass_begin_info.framebuffer));

    return _render_pass_begin_info.renderPass;
}

VkRenderPass VKFramebuffer::Stub::begin(VkCommandBuffer commandBuffer)
{
    vkCmdBeginRenderPass(commandBuffer, &_render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdSetViewport(commandBuffer, 0, 1, &_viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &_scissor);
    return _render_pass_begin_info.renderPass;
}

}

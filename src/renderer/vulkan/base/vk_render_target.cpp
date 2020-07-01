#include "renderer/vulkan/base/vk_render_target.h"

#include <array>

#include "graphics/base/color.h"

#include "renderer/base/render_engine_context.h"

#include "renderer/vulkan/base/vk_device.h"
#include "renderer/vulkan/base/vk_command_pool.h"
#include "renderer/vulkan/base/vk_descriptor_pool.h"
#include "renderer/vulkan/base/vk_graphics_context.h"
#include "renderer/vulkan/util/vk_util.h"

#ifdef ARK_PLATFORM_DARWIN
struct NSView;
#endif

namespace ark {

namespace vulkan {

VKRenderTarget::VKRenderTarget(const RenderEngineContext& renderContext, sp<VKDevice>& device)
    : _device(device), _clear_values{}, _render_pass_begin_info(vks::initializers::renderPassBeginInfo()), _viewport{}, _aquired_image_id(0)
{
    _swap_chain.connect(_device->vkInstance(), _device->vkPhysicalDevice(), _device->vkLogicalDevice());

    _clear_values[0].color = {{0, 0, 0, 0}};
    _clear_values[1].depthStencil = {1.0f, 0};

    _render_pass_begin_info.renderArea.offset.x = 0;
    _render_pass_begin_info.renderArea.offset.y = 0;
    _render_pass_begin_info.clearValueCount = 2;
    _render_pass_begin_info.pClearValues = _clear_values;

    initSwapchain(renderContext);

    _queue = _device->getQueueByFamilyIndex(_swap_chain.queueNodeIndex);
    _command_pool = sp<VKCommandPool>::make(_device, _swap_chain.queueNodeIndex);
}

VKRenderTarget::~VKRenderTarget()
{
    VkDevice logicalDevice = _device->vkLogicalDevice();
    vkDestroyRenderPass(logicalDevice, _render_pass_begin_info.renderPass, nullptr);

    for(size_t i = 0; i < _frame_buffers.size(); i++)
        vkDestroyFramebuffer(logicalDevice, _frame_buffers[i], nullptr);

    vkDestroyImageView(logicalDevice, _depth_stencil.view, nullptr);
    vkDestroyImage(logicalDevice, _depth_stencil.image, nullptr);
    vkFreeMemory(logicalDevice, _depth_stencil.mem, nullptr);

    _swap_chain.cleanup();
}

uint32_t VKRenderTarget::width() const
{
    return _width;
}

uint32_t VKRenderTarget::height() const
{
    return _height;
}

const VkRect2D& VKRenderTarget::vkScissor() const
{
    return _scissor;
}

const VkViewport& VKRenderTarget::vkViewport() const
{
    return _viewport;
}

const VkRenderPassBeginInfo& VKRenderTarget::vkRenderPassBeginInfo() const
{
    return _render_pass_begin_info;
}

const sp<VKDevice>& VKRenderTarget::device() const
{
    return _device;
}

const sp<VKCommandPool>& VKRenderTarget::commandPool() const
{
    return _command_pool;
}

VkRenderPass VKRenderTarget::vkRenderPass() const
{
    return _render_pass_begin_info.renderPass;
}

const std::vector<VkFramebuffer>& VKRenderTarget::frameBuffers() const
{
    return _frame_buffers;
}

std::vector<VkCommandBuffer> VKRenderTarget::makeCommandBuffers() const
{
    return _command_pool->makeCommandBuffers(_swap_chain.imageCount);
}

sp<VKDescriptorPool> VKRenderTarget::makeDescriptorPool(const sp<Recycler>& recycler) const
{
    VkDescriptorPool descriptorPool;

    std::vector<VkDescriptorPoolSize> poolSizes = {
        vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, _swap_chain.imageCount),
        vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, _swap_chain.imageCount)
    };

    VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, 1);
    VKUtil::checkResult(vkCreateDescriptorPool(_device->vkLogicalDevice(), &descriptorPoolInfo, nullptr, &descriptorPool));

    return sp<VKDescriptorPool>::make(recycler, _device, descriptorPool);
}

uint32_t VKRenderTarget::acquire(VKGraphicsContext& vkContext)
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);
    _submit_queue.clear();
    VkResult result = _swap_chain.acquireNextImage(vkContext._semaphore_present_complete, &_aquired_image_id);
    if(result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        onSurfaceChanged(_width, _height);
        result = _swap_chain.acquireNextImage(vkContext._semaphore_present_complete, &_aquired_image_id);
    }
    VKUtil::checkResult(result);
    return _aquired_image_id;
}

uint32_t VKRenderTarget::aquiredImageId() const
{
    return _aquired_image_id;
}

void VKRenderTarget::submit(VkCommandBuffer commandBuffer)
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);
    _submit_queue.push_back(commandBuffer);
}

void VKRenderTarget::swap(VKGraphicsContext& vkContext)
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);

    std::vector<VkSubmitInfo>& submitInfos = vkContext.submitInfos();

    VkSubmitInfo& submitInfo = submitInfos[0];
    submitInfo.pCommandBuffers = _submit_queue.data();
    submitInfo.commandBufferCount = static_cast<uint32_t>(_submit_queue.size());
    VKUtil::checkResult(vkQueueSubmit(_queue, submitInfos.size(), &submitInfo, VK_NULL_HANDLE));

    VKUtil::checkResult(_swap_chain.queuePresent(_queue, _aquired_image_id, vkContext._semaphore_render_complete));
    vkQueueWaitIdle(_queue);
}

void VKRenderTarget::onSurfaceChanged(uint32_t width, uint32_t height)
{
    _width = width;
    _height = height;
    _swap_chain.create(&_width, &_height, true);
    setupDepthStencil();
    setupRenderPass();
    setupFrameBuffer();

    _scissor = vks::initializers::rect2D(static_cast<int32_t>(_width), static_cast<int32_t>(_height), 0, 0);
    _viewport = vks::initializers::viewport(static_cast<float>(_width), static_cast<float>(_height), 0.0f, 1.0f);

    _render_pass_begin_info.renderArea.extent.width = _width;
    _render_pass_begin_info.renderArea.extent.height = _height;
}

void VKRenderTarget::initSwapchain(const RenderEngineContext& renderContext)
{
    const RenderEngineContext::Info& info = renderContext.info();
#if defined(_WIN32)
    _swap_chain.initSurface(info.windows.hinstance, info.windows.window);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    _swap_chain.initSurface(info.android.window);
#elif (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
    _swap_chain.initSurface(info.darwin.view);
#elif defined(_DIRECT2DISPLAY)
    _swap_chain.initSurface(width, height);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    _swap_chain.initSurface(display, surface);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    _swap_chain.initSurface(connection, window);
#endif
}

void VKRenderTarget::setupDepthStencil()
{
    VkImageCreateInfo image = {};
    image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image.pNext = nullptr;
    image.imageType = VK_IMAGE_TYPE_2D;
    image.format = _device->vkDepthFormat();
    image.extent = { _width, _height, 1 };
    image.mipLevels = 1;
    image.arrayLayers = 1;
    image.samples = VK_SAMPLE_COUNT_1_BIT;
    image.tiling = VK_IMAGE_TILING_OPTIMAL;
    image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image.flags = 0;

    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = nullptr;
    mem_alloc.allocationSize = 0;
    mem_alloc.memoryTypeIndex = 0;

    VkImageViewCreateInfo depthStencilView = {};
    depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    depthStencilView.pNext = nullptr;
    depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depthStencilView.format = _device->vkDepthFormat();
    depthStencilView.flags = 0;
    depthStencilView.subresourceRange = {};
    depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    depthStencilView.subresourceRange.baseMipLevel = 0;
    depthStencilView.subresourceRange.levelCount = 1;
    depthStencilView.subresourceRange.baseArrayLayer = 0;
    depthStencilView.subresourceRange.layerCount = 1;

    VkMemoryRequirements memReqs;

    VKUtil::checkResult(vkCreateImage(_device->vkLogicalDevice(), &image, nullptr, &_depth_stencil.image));
    vkGetImageMemoryRequirements(_device->vkLogicalDevice(), _depth_stencil.image, &memReqs);
    mem_alloc.allocationSize = memReqs.size;
    mem_alloc.memoryTypeIndex = _device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VKUtil::checkResult(vkAllocateMemory(_device->vkLogicalDevice(), &mem_alloc, nullptr, &_depth_stencil.mem));
    VKUtil::checkResult(vkBindImageMemory(_device->vkLogicalDevice(), _depth_stencil.image, _depth_stencil.mem, 0));

    depthStencilView.image = _depth_stencil.image;
    VKUtil::checkResult(vkCreateImageView(_device->vkLogicalDevice(), &depthStencilView, nullptr, &_depth_stencil.view));
}

void VKRenderTarget::setupRenderPass()
{
    std::array<VkAttachmentDescription, 2> attachments = {};
    // Color attachment
    attachments[0].format = _swap_chain.colorFormat;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    // Depth attachment
    attachments[1].format = _device->vkDepthFormat();
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorReference = {};
    colorReference.attachment = 0;
    colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthReference = {};
    depthReference.attachment = 1;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorReference;
    subpassDescription.pDepthStencilAttachment = &depthReference;
    subpassDescription.inputAttachmentCount = 0;
    subpassDescription.pInputAttachments = nullptr;
    subpassDescription.preserveAttachmentCount = 0;
    subpassDescription.pPreserveAttachments = nullptr;
    subpassDescription.pResolveAttachments = nullptr;

    // Subpass dependencies for layout transitions
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

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDescription;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    VKUtil::checkResult(vkCreateRenderPass(_device->vkLogicalDevice(), &renderPassInfo, nullptr, &_render_pass_begin_info.renderPass));
}

void VKRenderTarget::setupFrameBuffer()
{
    VkImageView attachments[2];

    // Depth/Stencil attachment is the same for all frame buffers
    attachments[1] = _depth_stencil.view;

    VkFramebufferCreateInfo frameBufferCreateInfo = vks::initializers::framebufferCreateInfo();
    frameBufferCreateInfo.pNext = nullptr;
    frameBufferCreateInfo.renderPass = _render_pass_begin_info.renderPass;
    frameBufferCreateInfo.attachmentCount = 2;
    frameBufferCreateInfo.pAttachments = attachments;
    frameBufferCreateInfo.width = _width;
    frameBufferCreateInfo.height = _height;
    frameBufferCreateInfo.layers = 1;

    // Create frame buffers for every swap chain image
    _frame_buffers.resize(_swap_chain.imageCount);
    for (uint32_t i = 0; i < _frame_buffers.size(); i++)
    {
        attachments[0] = _swap_chain.buffers[i].view;
        VKUtil::checkResult(vkCreateFramebuffer(_device->vkLogicalDevice(), &frameBufferCreateInfo, nullptr, &_frame_buffers[i]));
    }
}

}
}

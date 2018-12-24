#include "renderer/vulkan/base/vk_render_target.h"

#include <array>

#include "renderer/vulkan/base/vk_device.h"

#include "renderer/vulkan/base/vk_command_pool.h"
#include "renderer/vulkan/base/vk_util.h"

namespace ark {

#ifdef _WIN32
    extern HINSTANCE gInstance;
    extern HWND gWnd;
#endif

namespace vulkan {

VKRenderTarget::VKRenderTarget(const sp<VKDevice>& device)
    : _device(device)
{
    _queue = _device->queue();
    _swap_chain.connect(_device->vkInstance(), _device->physicalDevice(), _device->logicalDevice());

    // Create synchronization objects
    VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
    // Create a semaphore used to synchronize image presentation
    // Ensures that the image is displayed before we start submitting new commands to the queu
    VKUtil::checkResult(vkCreateSemaphore(_device->logicalDevice(), &semaphoreCreateInfo, nullptr, &_semaphore_present_complete));
    // Create a semaphore used to synchronize command submission
    // Ensures that the image is not presented until all commands have been sumbitted and executed
    VKUtil::checkResult(vkCreateSemaphore(_device->logicalDevice(), &semaphoreCreateInfo, nullptr, &_semaphore_render_complete));

    // Set up submit info structure
    // Semaphores will stay the same during application lifetime
    // Command buffer submission info is set by each example
    _submit_info = vks::initializers::submitInfo();
    _submit_info.pWaitDstStageMask = &_submit_pipeline_stages;
    _submit_info.waitSemaphoreCount = 1;
    _submit_info.pWaitSemaphores = &_semaphore_present_complete;
    _submit_info.signalSemaphoreCount = 1;
    _submit_info.pSignalSemaphores = &_semaphore_render_complete;

    initSwapchain();

    _command_pool = sp<VKCommandPool>::make(_device, _swap_chain.queueNodeIndex);

    setupDepthStencil();
    setupRenderPass();
    setupFrameBuffer();

    setupDescriptorPool();
}

VKRenderTarget::~VKRenderTarget()
{
    VkDevice logicalDevice = _device->logicalDevice();
    vkDestroyRenderPass(logicalDevice, _render_pass, nullptr);

    for(size_t i = 0; i < _frame_buffers.size(); i++)
        vkDestroyFramebuffer(logicalDevice, _frame_buffers[i], nullptr);

    vkDestroyImageView(logicalDevice, _depth_stencil.view, nullptr);
    vkDestroyImage(logicalDevice, _depth_stencil.image, nullptr);
    vkFreeMemory(logicalDevice, _depth_stencil.mem, nullptr);

    vkDestroySemaphore(logicalDevice, _semaphore_present_complete, nullptr);
    vkDestroySemaphore(logicalDevice, _semaphore_render_complete, nullptr);

    _swap_chain.cleanup();

    vkDestroyDescriptorPool(_device->logicalDevice(), _descriptor_pool, nullptr);
}

uint32_t VKRenderTarget::width() const
{
    return _width;
}

uint32_t VKRenderTarget::height() const
{
    return _height;
}

VkDescriptorPool VKRenderTarget::descriptorPool() const
{
    return _descriptor_pool;
}

const sp<VKDevice>& VKRenderTarget::device() const
{
    return _device;
}

const sp<VKCommandPool>& VKRenderTarget::commandPool() const
{
    return _command_pool;
}

VkRenderPass VKRenderTarget::renderPass() const
{
    return _render_pass;
}

const std::vector<VkFramebuffer>& VKRenderTarget::frameBuffers() const
{
    return _frame_buffers;
}

std::vector<VkCommandBuffer> VKRenderTarget::makeCommandBuffers() const
{
    return _command_pool->makeCommandBuffers(_swap_chain.imageCount);
}

uint32_t VKRenderTarget::acquire() const
{
    uint32_t currentBuffer;
    VKUtil::checkResult(_swap_chain.acquireNextImage(_semaphore_present_complete, &currentBuffer));
    return currentBuffer;
}

void VKRenderTarget::submit(VkCommandBuffer* commandBuffer)
{
    _submit_info.pCommandBuffers = commandBuffer;
    _submit_info.commandBufferCount = 1;
    VKUtil::checkResult(vkQueueSubmit(_queue, 1, &_submit_info, VK_NULL_HANDLE));
}

void VKRenderTarget::swap(uint32_t currentBuffer)
{
    VKUtil::checkResult(_swap_chain.queuePresent(_queue, currentBuffer, _semaphore_render_complete));
    vkQueueWaitIdle(_queue);
}

void VKRenderTarget::initSwapchain()
{
#if defined(_WIN32)
    _swap_chain.initSurface(gInstance, gWnd);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    _swap_chain.initSurface(androidApp->window);
#elif (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
    _swap_chain.initSurface(view);
#elif defined(_DIRECT2DISPLAY)
    _swap_chain.initSurface(width, height);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    _swap_chain.initSurface(display, surface);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    _swap_chain.initSurface(connection, window);
#endif
    _swap_chain.create(&_width, &_height, true);
}

void VKRenderTarget::setupDepthStencil()
{
    VkImageCreateInfo image = {};
    image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image.pNext = nullptr;
    image.imageType = VK_IMAGE_TYPE_2D;
    image.format = _device->depthFormat();
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
    depthStencilView.format = _device->depthFormat();
    depthStencilView.flags = 0;
    depthStencilView.subresourceRange = {};
    depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    depthStencilView.subresourceRange.baseMipLevel = 0;
    depthStencilView.subresourceRange.levelCount = 1;
    depthStencilView.subresourceRange.baseArrayLayer = 0;
    depthStencilView.subresourceRange.layerCount = 1;

    VkMemoryRequirements memReqs;

    VKUtil::checkResult(vkCreateImage(_device->logicalDevice(), &image, nullptr, &_depth_stencil.image));
    vkGetImageMemoryRequirements(_device->logicalDevice(), _depth_stencil.image, &memReqs);
    mem_alloc.allocationSize = memReqs.size;
    mem_alloc.memoryTypeIndex = _device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VKUtil::checkResult(vkAllocateMemory(_device->logicalDevice(), &mem_alloc, nullptr, &_depth_stencil.mem));
    VKUtil::checkResult(vkBindImageMemory(_device->logicalDevice(), _depth_stencil.image, _depth_stencil.mem, 0));

    depthStencilView.image = _depth_stencil.image;
    VKUtil::checkResult(vkCreateImageView(_device->logicalDevice(), &depthStencilView, nullptr, &_depth_stencil.view));
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
    attachments[1].format = _device->depthFormat();
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

    VKUtil::checkResult(vkCreateRenderPass(_device->logicalDevice(), &renderPassInfo, nullptr, &_render_pass));
}

void VKRenderTarget::setupFrameBuffer()
{
    VkImageView attachments[2];

    // Depth/Stencil attachment is the same for all frame buffers
    attachments[1] = _depth_stencil.view;

    VkFramebufferCreateInfo frameBufferCreateInfo = {};
    frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferCreateInfo.pNext = nullptr;
    frameBufferCreateInfo.renderPass = _render_pass;
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
        VKUtil::checkResult(vkCreateFramebuffer(_device->logicalDevice(), &frameBufferCreateInfo, nullptr, &_frame_buffers[i]));
    }
}

void VKRenderTarget::setupDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> poolSizes = {
        vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
        vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
    };

    VkDescriptorPoolCreateInfo descriptorPoolInfo =
            vks::initializers::descriptorPoolCreateInfo(
                static_cast<uint32_t>(poolSizes.size()),
                poolSizes.data(),
                2);

    VKUtil::checkResult(vkCreateDescriptorPool(_device->logicalDevice(), &descriptorPoolInfo, nullptr, &_descriptor_pool));
}

}
}

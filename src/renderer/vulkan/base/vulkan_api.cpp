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

#include "renderer/vulkan/base/vulkan_tools.h"
#include "renderer/vulkan/base/vulkan_debug.h"

#include "generated/vulkan_plugin.h"

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
    vkDeviceWaitIdle(_device);
    vkDestroyPipeline(_device, _pipeline, nullptr);

    vkDestroyPipelineLayout(_device, pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(_device, descriptorSetLayout, nullptr);

    vkDestroyBuffer(_device, vertices.buffer, nullptr);
    vkFreeMemory(_device, vertices.memory, nullptr);

    vkDestroyBuffer(_device, indices.buffer, nullptr);
    vkFreeMemory(_device, indices.memory, nullptr);

    vkDestroyBuffer(_device, uniformBufferVS.buffer, nullptr);
    vkFreeMemory(_device, uniformBufferVS.memory, nullptr);

    destroyTextureImage(texture);
//--------------------------------------------------------
    // Clean up Vulkan resources
    _swap_chain.cleanup();
    if (descriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(_device, descriptorPool, nullptr);
    }
    destroyCommandBuffers();
    vkDestroyRenderPass(_device, _render_pass, nullptr);
    for (uint32_t i = 0; i < _frame_buffers.size(); i++)
    {
        vkDestroyFramebuffer(_device, _frame_buffers[i], nullptr);
    }

    vkDestroyImageView(_device, _depth_stencil.view, nullptr);
    vkDestroyImage(_device, _depth_stencil.image, nullptr);
    vkFreeMemory(_device, _depth_stencil.mem, nullptr);

    vkDestroyPipelineCache(_device, _pipeline_cache, nullptr);

    vkDestroyCommandPool(_device, _command_pool, nullptr);

    vkDestroySemaphore(_device, _semaphores.present_complete, nullptr);
    vkDestroySemaphore(_device, _semaphores.render_complete, nullptr);

    _vulkan_device.reset();

    if (_VULKAN_VALIDATE)
    {
        vks::debug::freeDebugCallback(_instance);
    }

    vkDestroyInstance(_instance, nullptr);

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
    createInstance();
    if(_VULKAN_VALIDATE)
    {
        // The report flags determine what type of messages for the layers will be displayed
        // For validating (debugging) an appplication the error and warning bits should suffice
        VkDebugReportFlagsEXT debugReportFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        // Additional flags include performance info, loader and layer debug messages, etc.
        vks::debug::setupDebugging(_instance, debugReportFlags, VK_NULL_HANDLE);
    }
    createDevice();

    initSwapchain();
    createCommandPool();
    createCommandBuffers();
    setupDepthStencil();
    setupRenderPass();
    createPipelineCache();
    setupFrameBuffer();

    prepareVertices(true);
    prepareUniformBuffers();
    setupDescriptorSetLayout();
    preparePipelines();
    setupDescriptorPool();
    setupDescriptorSet();
    buildCommandBuffers();
    loadTexture();
    prepared = true;
}

void VulkanAPI::render()
{
    if (!prepared)
        return;
    draw();
}

void VulkanAPI::createInstance()
{
    // Validation can also be forced via a define
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "ark";
    appInfo.pEngineName = "ark";
    appInfo.apiVersion = VK_API_VERSION_1_0;

    std::vector<const char*> instanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };

    // Enable surface extensions depending on os
#if defined(_WIN32)
    instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    instanceExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(_DIRECT2DISPLAY)
    instanceExtensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    instanceExtensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    instanceExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
    instanceExtensions.push_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
    instanceExtensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#endif

    if (_instance_extensions.size() > 0) {
        for (auto enabledExtension : _instance_extensions) {
            instanceExtensions.push_back(enabledExtension);
        }
    }

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = nullptr;
    instanceCreateInfo.pApplicationInfo = &appInfo;

    if(_VULKAN_VALIDATE)
    {
        instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();

    if (_VULKAN_VALIDATE)
    {
        instanceCreateInfo.enabledLayerCount = vks::debug::validationLayerCount;
        instanceCreateInfo.ppEnabledLayerNames = vks::debug::validationLayerNames;
    }

    checkResult(vkCreateInstance(&instanceCreateInfo, nullptr, &_instance));
}

void VulkanAPI::createDevice()
{
    // Physical device
    uint32_t gpuCount = 0;
    // Get number of available physical devices
    checkResult(vkEnumeratePhysicalDevices(_instance, &gpuCount, nullptr));
    DASSERT(gpuCount > 0);
    // Enumerate devices
    std::vector<VkPhysicalDevice> physicalDevices(gpuCount);

    checkResult(vkEnumeratePhysicalDevices(_instance, &gpuCount, physicalDevices.data()));

    // GPU selection

    // Select physical device to be used for the Vulkan example
    // Defaults to the first device unless specified by command line
    uint32_t selectedDevice = 0;

    _physical_device = physicalDevices[selectedDevice];

    // Store properties (including limits), features and memory properties of the phyiscal device (so that examples can check against them)
    vkGetPhysicalDeviceProperties(_physical_device, &_device_properties);
    vkGetPhysicalDeviceFeatures(_physical_device, &_device_features);
    vkGetPhysicalDeviceMemoryProperties(_physical_device, &_device_memory_properties);

    if (_device_features.samplerAnisotropy) {
        _enabled_features.samplerAnisotropy = VK_TRUE;
    };

    // Vulkan device creation
    // This is handled by a separate class that gets a logical device representation
    // and encapsulates functions related to a device
    _vulkan_device.reset(new vks::VulkanDevice(_physical_device));
    checkResult(_vulkan_device->createLogicalDevice(_enabled_features, _enabled_device_extensions));
    _device = _vulkan_device->logicalDevice;

    // Get a graphics queue from the device
    vkGetDeviceQueue(_device, _vulkan_device->queueFamilyIndices.graphics, 0, &_queue);

    // Find a suitable depth format
    VkBool32 validDepthFormat = vks::tools::getSupportedDepthFormat(_physical_device, &_depth_format);
    DASSERT(validDepthFormat);

    _swap_chain.connect(_instance, _physical_device, _device);

    // Create synchronization objects
    VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
    // Create a semaphore used to synchronize image presentation
    // Ensures that the image is displayed before we start submitting new commands to the queu
    checkResult(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_semaphores.present_complete));
    // Create a semaphore used to synchronize command submission
    // Ensures that the image is not presented until all commands have been sumbitted and executed
    checkResult(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_semaphores.render_complete));

    // Set up submit info structure
    // Semaphores will stay the same during application lifetime
    // Command buffer submission info is set by each example
    _submit_info = vks::initializers::submitInfo();
    _submit_info.pWaitDstStageMask = &_submit_pipeline_stages;
    _submit_info.waitSemaphoreCount = 1;
    _submit_info.pWaitSemaphores = &_semaphores.present_complete;
    _submit_info.signalSemaphoreCount = 1;
    _submit_info.pSignalSemaphores = &_semaphores.render_complete;
}

void VulkanAPI::initSwapchain()
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

void VulkanAPI::createCommandPool()
{
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = _swap_chain.queueNodeIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    checkResult(vkCreateCommandPool(_device, &cmdPoolInfo, nullptr, &_command_pool));
}

void VulkanAPI::createCommandBuffers()
{
    // Create one command buffer for each swap chain image and reuse for rendering
    _command_buffers.resize(_swap_chain.imageCount);

    VkCommandBufferAllocateInfo cmdBufAllocateInfo =
            vks::initializers::commandBufferAllocateInfo(
                _command_pool,
                VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                static_cast<uint32_t>(_command_buffers.size()));

    checkResult(vkAllocateCommandBuffers(_device, &cmdBufAllocateInfo, _command_buffers.data()));
}

void VulkanAPI::setupDepthStencil()
{
    VkImageCreateInfo image = {};
    image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image.pNext = nullptr;
    image.imageType = VK_IMAGE_TYPE_2D;
    image.format = _depth_format;
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
    depthStencilView.format = _depth_format;
    depthStencilView.flags = 0;
    depthStencilView.subresourceRange = {};
    depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    depthStencilView.subresourceRange.baseMipLevel = 0;
    depthStencilView.subresourceRange.levelCount = 1;
    depthStencilView.subresourceRange.baseArrayLayer = 0;
    depthStencilView.subresourceRange.layerCount = 1;

    VkMemoryRequirements memReqs;

    checkResult(vkCreateImage(_device, &image, nullptr, &_depth_stencil.image));
    vkGetImageMemoryRequirements(_device, _depth_stencil.image, &memReqs);
    mem_alloc.allocationSize = memReqs.size;
    mem_alloc.memoryTypeIndex = _vulkan_device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    checkResult(vkAllocateMemory(_device, &mem_alloc, nullptr, &_depth_stencil.mem));
    checkResult(vkBindImageMemory(_device, _depth_stencil.image, _depth_stencil.mem, 0));

    depthStencilView.image = _depth_stencil.image;
    checkResult(vkCreateImageView(_device, &depthStencilView, nullptr, &_depth_stencil.view));
}

void VulkanAPI::setupRenderPass()
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
    attachments[1].format = _depth_format;
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

    checkResult(vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_render_pass));
}

void VulkanAPI::createPipelineCache()
{
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    checkResult(vkCreatePipelineCache(_device, &pipelineCacheCreateInfo, nullptr, &_pipeline_cache));
}

void VulkanAPI::setupFrameBuffer()
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
        checkResult(vkCreateFramebuffer(_device, &frameBufferCreateInfo, nullptr, &_frame_buffers[i]));
    }
}

void VulkanAPI::prepareVertices(bool useStagingBuffers)
{
    // A note on memory management in Vulkan in general:
    //	This is a very complex topic and while it's fine for an example application to to small individual memory allocations that is not
    //	what should be done a real-world application, where you should allocate large chunkgs of memory at once isntead.

    // Setup vertices
    std::vector<Vertex> vertexBuffer =
    {
        { {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
        { { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
        { {  0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
    };
    uint32_t vertexBufferSize = static_cast<uint32_t>(vertexBuffer.size()) * sizeof(Vertex);

    // Setup indices
    std::vector<uint32_t> indexBuffer = { 0, 1, 2 };
    indices.count = static_cast<uint32_t>(indexBuffer.size());
    uint32_t indexBufferSize = indices.count * sizeof(uint32_t);

    VkMemoryAllocateInfo memAlloc = {};
    memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    VkMemoryRequirements memReqs;

    void *data;

    if (useStagingBuffers)
    {
        // Static data like vertex and index buffer should be stored on the device memory
        // for optimal (and fastest) access by the GPU
        //
        // To achieve this we use so-called "staging buffers" :
        // - Create a buffer that's visible to the host (and can be mapped)
        // - Copy the data to this buffer
        // - Create another buffer that's local on the device (VRAM) with the same size
        // - Copy the data from the host to the device using a command buffer
        // - Delete the host visible (staging) buffer
        // - Use the device local buffers for rendering

        struct StagingBuffer {
            VkDeviceMemory memory;
            VkBuffer buffer;
        };

        struct {
            StagingBuffer vertices;
            StagingBuffer indices;
        } stagingBuffers;

        // Vertex buffer
        VkBufferCreateInfo vertexBufferInfo = {};
        vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vertexBufferInfo.size = vertexBufferSize;
        // Buffer is used as the copy source
        vertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        // Create a host-visible buffer to copy the vertex data to (staging buffer)
        checkResult(vkCreateBuffer(_device, &vertexBufferInfo, nullptr, &stagingBuffers.vertices.buffer));
        vkGetBufferMemoryRequirements(_device, stagingBuffers.vertices.buffer, &memReqs);
        memAlloc.allocationSize = memReqs.size;
        // Request a host visible memory type that can be used to copy our data do
        // Also request it to be coherent, so that writes are visible to the GPU right after unmapping the buffer
        memAlloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        checkResult(vkAllocateMemory(_device, &memAlloc, nullptr, &stagingBuffers.vertices.memory));
        // Map and copy
        checkResult(vkMapMemory(_device, stagingBuffers.vertices.memory, 0, memAlloc.allocationSize, 0, &data));
        memcpy(data, vertexBuffer.data(), vertexBufferSize);
        vkUnmapMemory(_device, stagingBuffers.vertices.memory);
        checkResult(vkBindBufferMemory(_device, stagingBuffers.vertices.buffer, stagingBuffers.vertices.memory, 0));

        // Create a device local buffer to which the (host local) vertex data will be copied and which will be used for rendering
        vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        checkResult(vkCreateBuffer(_device, &vertexBufferInfo, nullptr, &vertices.buffer));
        vkGetBufferMemoryRequirements(_device, vertices.buffer, &memReqs);
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        checkResult(vkAllocateMemory(_device, &memAlloc, nullptr, &vertices.memory));
        checkResult(vkBindBufferMemory(_device, vertices.buffer, vertices.memory, 0));

        // Index buffer
        VkBufferCreateInfo indexbufferInfo = {};
        indexbufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        indexbufferInfo.size = indexBufferSize;
        indexbufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        // Copy index data to a buffer visible to the host (staging buffer)
        checkResult(vkCreateBuffer(_device, &indexbufferInfo, nullptr, &stagingBuffers.indices.buffer));
        vkGetBufferMemoryRequirements(_device, stagingBuffers.indices.buffer, &memReqs);
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        checkResult(vkAllocateMemory(_device, &memAlloc, nullptr, &stagingBuffers.indices.memory));
        checkResult(vkMapMemory(_device, stagingBuffers.indices.memory, 0, indexBufferSize, 0, &data));
        memcpy(data, indexBuffer.data(), indexBufferSize);
        vkUnmapMemory(_device, stagingBuffers.indices.memory);
        checkResult(vkBindBufferMemory(_device, stagingBuffers.indices.buffer, stagingBuffers.indices.memory, 0));

        // Create destination buffer with device only visibility
        indexbufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        checkResult(vkCreateBuffer(_device, &indexbufferInfo, nullptr, &indices.buffer));
        vkGetBufferMemoryRequirements(_device, indices.buffer, &memReqs);
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        checkResult(vkAllocateMemory(_device, &memAlloc, nullptr, &indices.memory));
        checkResult(vkBindBufferMemory(_device, indices.buffer, indices.memory, 0));

        // Buffer copies have to be submitted to a queue, so we need a command buffer for them
        // Note: Some devices offer a dedicated transfer queue (with only the transfer bit set) that may be faster when doing lots of copies
        VkCommandBuffer copyCmd = getCommandBuffer(true);

        // Put buffer region copies into command buffer
        VkBufferCopy copyRegion = {};

        // Vertex buffer
        copyRegion.size = vertexBufferSize;
        vkCmdCopyBuffer(copyCmd, stagingBuffers.vertices.buffer, vertices.buffer, 1, &copyRegion);
        // Index buffer
        copyRegion.size = indexBufferSize;
        vkCmdCopyBuffer(copyCmd, stagingBuffers.indices.buffer, indices.buffer,	1, &copyRegion);

        // Flushing the command buffer will also submit it to the queue and uses a fence to ensure that all commands have been executed before returning
        flushCommandBuffer(copyCmd);

        // Destroy staging buffers
        // Note: Staging buffer must not be deleted before the copies have been submitted and executed
        vkDestroyBuffer(_device, stagingBuffers.vertices.buffer, nullptr);
        vkFreeMemory(_device, stagingBuffers.vertices.memory, nullptr);
        vkDestroyBuffer(_device, stagingBuffers.indices.buffer, nullptr);
        vkFreeMemory(_device, stagingBuffers.indices.memory, nullptr);
    }
    else
    {
        // Don't use staging
        // Create host-visible buffers only and use these for rendering. This is not advised and will usually result in lower rendering performance

        // Vertex buffer
        VkBufferCreateInfo vertexBufferInfo = {};
        vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vertexBufferInfo.size = vertexBufferSize;
        vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        // Copy vertex data to a buffer visible to the host
        checkResult(vkCreateBuffer(_device, &vertexBufferInfo, nullptr, &vertices.buffer));
        vkGetBufferMemoryRequirements(_device, vertices.buffer, &memReqs);
        memAlloc.allocationSize = memReqs.size;
        // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT is host visible memory, and VK_MEMORY_PROPERTY_HOST_COHERENT_BIT makes sure writes are directly visible
        memAlloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        checkResult(vkAllocateMemory(_device, &memAlloc, nullptr, &vertices.memory));
        checkResult(vkMapMemory(_device, vertices.memory, 0, memAlloc.allocationSize, 0, &data));
        memcpy(data, vertexBuffer.data(), vertexBufferSize);
        vkUnmapMemory(_device, vertices.memory);
        checkResult(vkBindBufferMemory(_device, vertices.buffer, vertices.memory, 0));

        // Index buffer
        VkBufferCreateInfo indexbufferInfo = {};
        indexbufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        indexbufferInfo.size = indexBufferSize;
        indexbufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

        // Copy index data to a buffer visible to the host
        checkResult(vkCreateBuffer(_device, &indexbufferInfo, nullptr, &indices.buffer));
        vkGetBufferMemoryRequirements(_device, indices.buffer, &memReqs);
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        checkResult(vkAllocateMemory(_device, &memAlloc, nullptr, &indices.memory));
        checkResult(vkMapMemory(_device, indices.memory, 0, indexBufferSize, 0, &data));
        memcpy(data, indexBuffer.data(), indexBufferSize);
        vkUnmapMemory(_device, indices.memory);
        checkResult(vkBindBufferMemory(_device, indices.buffer, indices.memory, 0));
    }
}

void VulkanAPI::prepareUniformBuffers()
{
    // Prepare and initialize a uniform buffer block containing shader uniforms
    // Single uniforms like in OpenGL are no longer present in Vulkan. All Shader uniforms are passed via uniform buffer blocks
    VkMemoryRequirements memReqs;

    // Vertex shader uniform buffer block
    VkBufferCreateInfo bufferInfo = {};
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.allocationSize = 0;
    allocInfo.memoryTypeIndex = 0;

    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(uboVS);
    // This buffer will be used as a uniform buffer
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    // Create a new buffer
    checkResult(vkCreateBuffer(_device, &bufferInfo, nullptr, &uniformBufferVS.buffer));
    // Get memory requirements including size, alignment and memory type
    vkGetBufferMemoryRequirements(_device, uniformBufferVS.buffer, &memReqs);
    allocInfo.allocationSize = memReqs.size;
    // Get the memory type index that supports host visibile memory access
    // Most implementations offer multiple memory types and selecting the correct one to allocate memory from is crucial
    // We also want the buffer to be host coherent so we don't have to flush (or sync after every update.
    // Note: This may affect performance so you might not want to do this in a real world application that updates buffers on a regular base
    allocInfo.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    // Allocate memory for the uniform buffer
    checkResult(vkAllocateMemory(_device, &allocInfo, nullptr, &(uniformBufferVS.memory)));
    // Bind memory to buffer
    checkResult(vkBindBufferMemory(_device, uniformBufferVS.buffer, uniformBufferVS.memory, 0));

    // Store information in the uniform's descriptor that is used by the descriptor set
    uniformBufferVS.descriptor.buffer = uniformBufferVS.buffer;
    uniformBufferVS.descriptor.offset = 0;
    uniformBufferVS.descriptor.range = sizeof(uboVS);

    updateUniformBuffers();
}

void VulkanAPI::updateUniformBuffers()
{
    // Update matrices
    uboVS.projectionMatrix = glm::perspective(glm::radians(60.0f), (float)_width / (float)_height, 0.1f, 256.0f);

    uboVS.viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, zoom));

    uboVS.modelMatrix = glm::mat4(1.0f);
    uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    // Map uniform buffer and update it
    uint8_t *pData;
    checkResult(vkMapMemory(_device, uniformBufferVS.memory, 0, sizeof(uboVS), 0, (void **)&pData));
    memcpy(pData, &uboVS, sizeof(uboVS));
    // Unmap after data has been copied
    // Note: Since we requested a host coherent memory type for the uniform buffer, the write is instantly visible to the GPU
    vkUnmapMemory(_device, uniformBufferVS.memory);
}

void VulkanAPI::setupDescriptorSetLayout()
{
    // Setup layout of descriptors used in this example
    // Basically connects the different shader stages to descriptors for binding uniform buffers, image samplers, etc.
    // So every shader binding should map to one descriptor set layout binding

    // Binding 0: Uniform buffer (Vertex shader)
    VkDescriptorSetLayoutBinding layoutBinding = {};
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
    descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorLayout.pNext = nullptr;
    descriptorLayout.bindingCount = 1;
    descriptorLayout.pBindings = &layoutBinding;

    checkResult(vkCreateDescriptorSetLayout(_device, &descriptorLayout, nullptr, &descriptorSetLayout));

    // Create the pipeline layout that is used to generate the rendering pipelines that are based on this descriptor set layout
    // In a more complex scenario you would have different pipeline layouts for different descriptor set layouts that could be reused
    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
    pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pPipelineLayoutCreateInfo.pNext = nullptr;
    pPipelineLayoutCreateInfo.setLayoutCount = 1;
    pPipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;

    checkResult(vkCreatePipelineLayout(_device, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout));
}

void VulkanAPI::preparePipelines()
{
    // Create the graphics pipeline used in this example
    // Vulkan uses the concept of rendering pipelines to encapsulate fixed states, replacing OpenGL's complex state machine
    // A pipeline is then stored and hashed on the GPU making pipeline changes very fast
    // Note: There are still a few dynamic states that are not directly part of the pipeline (but the info that they are used is)

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    // The layout used for this pipeline (can be shared among multiple pipelines using the same layout)
    pipelineCreateInfo.layout = pipelineLayout;
    // Renderpass this pipeline is attached to
    pipelineCreateInfo.renderPass = _render_pass;

    // Construct the differnent states making up the pipeline

    // Input assembly state describes how primitives are assembled
    // This pipeline will assemble vertex data as a triangle lists (though we only use one triangle)
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // Rasterization state
    VkPipelineRasterizationStateCreateInfo rasterizationState = {};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationState.cullMode = VK_CULL_MODE_NONE;
    rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationState.depthClampEnable = VK_FALSE;
    rasterizationState.rasterizerDiscardEnable = VK_FALSE;
    rasterizationState.depthBiasEnable = VK_FALSE;
    rasterizationState.lineWidth = 1.0f;

    // Color blend state describes how blend factors are calculated (if used)
    // We need one blend attachment state per color attachment (even if blending is not used
    VkPipelineColorBlendAttachmentState blendAttachmentState[1] = {};
    blendAttachmentState[0].colorWriteMask = 0xf;
    blendAttachmentState[0].blendEnable = VK_FALSE;
    VkPipelineColorBlendStateCreateInfo colorBlendState = {};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = blendAttachmentState;

    // Viewport state sets the number of viewports and scissor used in this pipeline
    // Note: This is actually overriden by the dynamic states (see below)
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // Enable dynamic states
    // Most states are baked into the pipeline, but there are still a few dynamic states that can be changed within a command buffer
    // To be able to change these we need do specify which dynamic states will be changed using this pipeline. Their actual states are set later on in the command buffer.
    // For this example we will set the viewport and scissor using dynamic states
    std::vector<VkDynamicState> dynamicStateEnables;
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pDynamicStates = dynamicStateEnables.data();
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

    // Depth and stencil state containing depth and stencil compare and test operations
    // We only use depth tests and want depth tests and writes to be enabled and compare with less or equal
    VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.depthTestEnable = VK_TRUE;
    depthStencilState.depthWriteEnable = VK_TRUE;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilState.depthBoundsTestEnable = VK_FALSE;
    depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
    depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
    depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
    depthStencilState.stencilTestEnable = VK_FALSE;
    depthStencilState.front = depthStencilState.back;

    // Multi sampling state
    // This example does not make use fo multi sampling (for anti-aliasing), the state must still be set and passed to the pipeline
    VkPipelineMultisampleStateCreateInfo multisampleState = {};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleState.pSampleMask = nullptr;

    // Vertex input descriptions
    // Specifies the vertex input parameters for a pipeline

    // Vertex input binding
    // This example uses a single vertex input binding at binding point 0 (see vkCmdBindVertexBuffers)
    VkVertexInputBindingDescription vertexInputBinding = {};
    vertexInputBinding.binding = 0;
    vertexInputBinding.stride = sizeof(Vertex);
    vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // Inpute attribute bindings describe shader attribute locations and memory layouts
    std::array<VkVertexInputAttributeDescription, 2> vertexInputAttributs;
    // These match the following shader layout (see triangle.vert):
    //	layout (location = 0) in vec3 inPos;
    //	layout (location = 1) in vec3 inColor;
    // Attribute location 0: Position
    vertexInputAttributs[0].binding = 0;
    vertexInputAttributs[0].location = 0;
    // Position attribute is three 32 bit signed (SFLOAT) floats (R32 G32 B32)
    vertexInputAttributs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexInputAttributs[0].offset = offsetof(Vertex, position);
    // Attribute location 1: Color
    vertexInputAttributs[1].binding = 0;
    vertexInputAttributs[1].location = 1;
    // Color attribute is three 32 bit signed (SFLOAT) floats (R32 G32 B32)
    vertexInputAttributs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexInputAttributs[1].offset = offsetof(Vertex, color);

    // Vertex input state used for pipeline creation
    VkPipelineVertexInputStateCreateInfo vertexInputState = {};
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState.vertexBindingDescriptionCount = 1;
    vertexInputState.pVertexBindingDescriptions = &vertexInputBinding;
    vertexInputState.vertexAttributeDescriptionCount = 2;
    vertexInputState.pVertexAttributeDescriptions = vertexInputAttributs.data();

    // Shaders
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};

    // Vertex shader
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    // Set pipeline stage for this shader
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    // Load binary SPIR-V shader
    shaderStages[0].module = loadSPIRVShader("triangle.vert.spv");
    // Main entry point for the shader
    shaderStages[0].pName = "main";
    DASSERT(shaderStages[0].module != VK_NULL_HANDLE);

    // Fragment shader
    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    // Set pipeline stage for this shader
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    // Load binary SPIR-V shader
    shaderStages[1].module = loadSPIRVShader("triangle.frag.spv");
    // Main entry point for the shader
    shaderStages[1].pName = "main";
    DASSERT(shaderStages[1].module != VK_NULL_HANDLE);

    // Set pipeline shader stage info
    pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineCreateInfo.pStages = shaderStages.data();

    // Assign the pipeline states to the pipeline creation info structure
    pipelineCreateInfo.pVertexInputState = &vertexInputState;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDepthStencilState = &depthStencilState;
    pipelineCreateInfo.renderPass = _render_pass;
    pipelineCreateInfo.pDynamicState = &dynamicState;

    // Create rendering pipeline using the specified states
    checkResult(vkCreateGraphicsPipelines(_device, _pipeline_cache, 1, &pipelineCreateInfo, nullptr, &_pipeline));

    // Shader modules are no longer needed once the graphics pipeline has been created
    vkDestroyShaderModule(_device, shaderStages[0].module, nullptr);
    vkDestroyShaderModule(_device, shaderStages[1].module, nullptr);
}

void VulkanAPI::setupDescriptorPool()
{
    // We need to tell the API the number of max. requested descriptors per type
    VkDescriptorPoolSize typeCounts[1];
    // This example only uses one descriptor type (uniform buffer) and only requests one descriptor of this type
    typeCounts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    typeCounts[0].descriptorCount = 1;
    // For additional types you need to add new entries in the type count list
    // E.g. for two combined image samplers :
    // typeCounts[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    // typeCounts[1].descriptorCount = 2;

    // Create the global descriptor pool
    // All descriptors used in this example are allocated from this pool
    VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.pNext = nullptr;
    descriptorPoolInfo.poolSizeCount = 1;
    descriptorPoolInfo.pPoolSizes = typeCounts;
    // Set the max. number of descriptor sets that can be requested from this pool (requesting beyond this limit will result in an error)
    descriptorPoolInfo.maxSets = 1;

    checkResult(vkCreateDescriptorPool(_device, &descriptorPoolInfo, nullptr, &descriptorPool));
}

void VulkanAPI::setupDescriptorSet()
{
    // Allocate a new descriptor set from the global descriptor pool
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    checkResult(vkAllocateDescriptorSets(_device, &allocInfo, &descriptorSet));

    // Update the descriptor set determining the shader binding points
    // For every binding point used in a shader there needs to be one
    // descriptor set matching that binding point

    VkWriteDescriptorSet writeDescriptorSet = {};

    // Binding 0 : Uniform buffer
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = descriptorSet;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.pBufferInfo = &uniformBufferVS.descriptor;
    // Binds this uniform buffer to binding point 0
    writeDescriptorSet.dstBinding = 0;

    vkUpdateDescriptorSets(_device, 1, &writeDescriptorSet, 0, nullptr);
}

void VulkanAPI::buildCommandBuffers()
{
    VkCommandBufferBeginInfo cmdBufInfo = {};
    cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufInfo.pNext = nullptr;

    // Set clear values for all framebuffer attachments with loadOp set to clear
    // We use two attachments (color and depth) that are cleared at the start of the subpass and as such we need to set clear values for both
    VkClearValue clearValues[2];
    clearValues[0].color = { { 0.0f, 0.0f, 0.2f, 1.0f } };
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderPass = _render_pass;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = _width;
    renderPassBeginInfo.renderArea.extent.height = _height;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;

    for (size_t i = 0; i < _command_buffers.size(); ++i)
    {
        // Set target frame buffer
        renderPassBeginInfo.framebuffer = _frame_buffers[i];

        checkResult(vkBeginCommandBuffer(_command_buffers[i], &cmdBufInfo));

        // Start the first sub pass specified in our default render pass setup by the base class
        // This will clear the color and depth attachment
        vkCmdBeginRenderPass(_command_buffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Update dynamic viewport state
        VkViewport viewport = {};
        viewport.height = static_cast<float>(_height);
        viewport.width = static_cast<float>(_width);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(_command_buffers[i], 0, 1, &viewport);

        // Update dynamic scissor state
        VkRect2D scissor = {};
        scissor.extent.width = _width;
        scissor.extent.height = _height;
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        vkCmdSetScissor(_command_buffers[i], 0, 1, &scissor);

        // Bind descriptor sets describing shader binding points
        vkCmdBindDescriptorSets(_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

        // Bind the rendering pipeline
        // The pipeline (state object) contains all states of the rendering pipeline, binding it will set all the states specified at pipeline creation time
        vkCmdBindPipeline(_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);

        // Bind triangle vertex buffer (contains position and colors)
        VkDeviceSize offsets[1] = { 0 };
        vkCmdBindVertexBuffers(_command_buffers[i], 0, 1, &vertices.buffer, offsets);

        // Bind triangle index buffer
        vkCmdBindIndexBuffer(_command_buffers[i], indices.buffer, 0, VK_INDEX_TYPE_UINT32);

        // Draw indexed triangle
        vkCmdDrawIndexed(_command_buffers[i], indices.count, 1, 0, 0, 1);

        vkCmdEndRenderPass(_command_buffers[i]);

        // Ending the render pass will add an implicit barrier transitioning the frame buffer color attachment to
        // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR for presenting it to the windowing system

        checkResult(vkEndCommandBuffer(_command_buffers[i]));
    }
}

//void VulkanAPI::generateQuad()
//{
//    // Setup vertices for a single uv-mapped quad made from two triangles
//    std::vector<Vertex> vertices =
//    {
//        { {  1.0f,  1.0f, 0.0f }, { 1.0f, 1.0f },{ 0.0f, 0.0f, 1.0f } },
//        { { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f } },
//        { { -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } },
//        { {  1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } }
//    };

//    // Setup indices
//    std::vector<uint32_t> indices = { 0,1,2, 2,3,0 };
//    indexCount = static_cast<uint32_t>(indices.size());

//    // Create buffers
//    // For the sake of simplicity we won't stage the vertex data to the gpu memory
//    // Vertex buffer
//    checkResult(vulkanDevice->createBuffer(
//                        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
//                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
//                        &vertexBuffer,
//                        vertices.size() * sizeof(Vertex),
//                        vertices.data()));
//    // Index buffer
//    checkResult(vulkanDevice->createBuffer(
//                        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
//                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
//                        &indexBuffer,
//                        indices.size() * sizeof(uint32_t),
//                        indices.data()));
//}

void VulkanAPI::draw()
{
    // Get next image in the swap chain (back/front buffer)
    checkResult(_swap_chain.acquireNextImage(_semaphores.present_complete, &currentBuffer));

    _submit_info.pCommandBuffers = &_command_buffers[currentBuffer];					// Command buffers(s) to execute in this batch (submission)
    _submit_info.commandBufferCount = 1;												// One command buffer

    checkResult(vkQueueSubmit(_queue, 1, &_submit_info, VK_NULL_HANDLE));

    // Present the current buffer to the swap chain
    // Pass the semaphore signaled by the command buffer submission from the submit info as the wait semaphore for swap chain presentation
    // This ensures that the image is not presented to the windowing system until all commands have been submitted
    checkResult(_swap_chain.queuePresent(_queue, currentBuffer, _semaphores.render_complete));
    vkQueueWaitIdle(_queue);
}

void VulkanAPI::checkResult(VkResult result) const
{
    DCHECK(result == VK_SUCCESS, "Vulkan error: %s", vks::tools::errorString(result).c_str());
}

uint32_t VulkanAPI::getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties) const
{
    // Iterate over all memory types available for the device used in this example
    for (uint32_t i = 0; i < _device_memory_properties.memoryTypeCount; i++)
    {
        if ((typeBits & 1) == 1)
        {
            if ((_device_memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }
        typeBits >>= 1;
    }

    throw "Could not find a suitable memory type!";
}

VkCommandBuffer VulkanAPI::getCommandBuffer(bool begin) const
{
    VkCommandBuffer cmdBuffer;

    VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
    cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufAllocateInfo.commandPool = _command_pool;
    cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufAllocateInfo.commandBufferCount = 1;

    checkResult(vkAllocateCommandBuffers(_device, &cmdBufAllocateInfo, &cmdBuffer));

    // If requested, also start the new command buffer
    if (begin)
    {
        VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
        checkResult(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));
    }

    return cmdBuffer;
}

void VulkanAPI::flushCommandBuffer(VkCommandBuffer commandBuffer) const
{
    DASSERT(commandBuffer != VK_NULL_HANDLE);

    checkResult(vkEndCommandBuffer(commandBuffer));

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    // Create fence to ensure that the command buffer has finished executing
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = 0;
    VkFence fence;
    checkResult(vkCreateFence(_device, &fenceCreateInfo, nullptr, &fence));

    // Submit to the queue
    checkResult(vkQueueSubmit(_queue, 1, &submitInfo, fence));
    // Wait for the fence to signal that command buffer has finished executing
    checkResult(vkWaitForFences(_device, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));

    vkDestroyFence(_device, fence, nullptr);
    vkFreeCommandBuffers(_device, _command_pool, 1, &commandBuffer);
}

VkShaderModule VulkanAPI::loadSPIRVShader(std::string filename) const
{
    size_t shaderSize;
    char* shaderCode = nullptr;

#if defined(__ANDROID__)
    // Load shader from compressed asset
    AAsset* asset = AAssetManager_open(androidApp->activity->assetManager, filename.c_str(), AASSET_MODE_STREAMING);
    assert(asset);
    shaderSize = AAsset_getLength(asset);
    assert(shaderSize > 0);

    shaderCode = new char[shaderSize];
    AAsset_read(asset, shaderCode, shaderSize);
    AAsset_close(asset);
#else
    std::ifstream is(filename, std::ios::binary | std::ios::in | std::ios::ate);

    if (is.is_open())
    {
        shaderSize = is.tellg();
        is.seekg(0, std::ios::beg);
        // Copy file contents into a buffer
        shaderCode = new char[shaderSize];
        is.read(shaderCode, shaderSize);
        is.close();
        DASSERT(shaderSize > 0);
    }
#endif
    if (shaderCode)
    {
        // Create a new shader module that will be used for pipeline creation
        VkShaderModuleCreateInfo moduleCreateInfo{};
        moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleCreateInfo.codeSize = shaderSize;
        moduleCreateInfo.pCode = (uint32_t*)shaderCode;

        VkShaderModule shaderModule;
        checkResult(vkCreateShaderModule(_device, &moduleCreateInfo, nullptr, &shaderModule));

        delete[] shaderCode;

        return shaderModule;
    }
    else
    {
        std::cerr << "Error: Could not open shader file \"" << filename << "\"" << std::endl;
        return VK_NULL_HANDLE;
    }
}

void VulkanAPI::destroyCommandBuffers() const
{
    vkFreeCommandBuffers(_device, _command_pool, static_cast<uint32_t>(_command_buffers.size()), _command_buffers.data());
}

void VulkanAPI::loadTexture()
{
    const std::string filename = "texture.png";
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    const bitmap tex = _resource_manager->bitmapLoader()->get(filename);

    texture.width = tex->width();
    texture.height = tex->height();
    texture.mipLevels = 1;

    // We prefer using staging to copy the texture data to a device local optimal image
    VkBool32 useStaging = true;

    // Only use linear tiling if forced
    bool forceLinearTiling = false;
    if (forceLinearTiling) {
        // Don't use linear if format is not supported for (linear) shader sampling
        // Get device properites for the requested texture format
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(_physical_device, format, &formatProperties);
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
        checkResult(vkCreateBuffer(_device, &bufferCreateInfo, nullptr, &stagingBuffer));

        // Get memory requirements for the staging buffer (alignment, memory type bits)
        vkGetBufferMemoryRequirements(_device, stagingBuffer, &memReqs);
        memAllocInfo.allocationSize = memReqs.size;
        // Get memory type index for a host visible buffer
        memAllocInfo.memoryTypeIndex = _vulkan_device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        checkResult(vkAllocateMemory(_device, &memAllocInfo, nullptr, &stagingMemory));
        checkResult(vkBindBufferMemory(_device, stagingBuffer, stagingMemory, 0));

        // Copy texture data into host local staging buffer
        uint8_t *data;
        checkResult(vkMapMemory(_device, stagingMemory, 0, memReqs.size, 0, (void **)&data));
        memcpy(data, tex->bytes()->buf(), tex->bytes()->size());
        vkUnmapMemory(_device, stagingMemory);

        // Setup buffer copy regions for each mip level
        std::vector<VkBufferImageCopy> bufferCopyRegions;
        uint32_t offset = 0;

        for (uint32_t i = 0; i < texture.mipLevels; i++) {
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
        imageCreateInfo.mipLevels = texture.mipLevels;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        // Set initial layout of the image to undefined
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.extent = { texture.width, texture.height, 1 };
        imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        checkResult(vkCreateImage(_device, &imageCreateInfo, nullptr, &texture.image));

        vkGetImageMemoryRequirements(_device, texture.image, &memReqs);
        memAllocInfo.allocationSize = memReqs.size;
        memAllocInfo.memoryTypeIndex = _vulkan_device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        checkResult(vkAllocateMemory(_device, &memAllocInfo, nullptr, &texture.deviceMemory));
        checkResult(vkBindImageMemory(_device, texture.image, texture.deviceMemory, 0));

        VkCommandBuffer copyCmd = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

        // Image memory barriers for the texture image

        // The sub resource range describes the regions of the image that will be transitioned using the memory barriers below
        VkImageSubresourceRange subresourceRange = {};
        // Image only contains color data
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        // Start at first mip level
        subresourceRange.baseMipLevel = 0;
        // We will transition on all mip levels
        subresourceRange.levelCount = texture.mipLevels;
        // The 2D texture only has one layer
        subresourceRange.layerCount = 1;

        // Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
        VkImageMemoryBarrier imageMemoryBarrier = vks::initializers::imageMemoryBarrier();;
        imageMemoryBarrier.image = texture.image;
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
                    texture.image,
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
        texture.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        flushCommandBuffer(copyCmd, _queue, true);

        // Clean up staging resources
        vkFreeMemory(_device, stagingMemory, nullptr);
        vkDestroyBuffer(_device, stagingBuffer, nullptr);
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
        imageCreateInfo.extent = { texture.width, texture.height, 1 };
        checkResult(vkCreateImage(_device, &imageCreateInfo, nullptr, &mappableImage));

        // Get memory requirements for this image like size and alignment
        vkGetImageMemoryRequirements(_device, mappableImage, &memReqs);
        // Set memory allocation size to required memory size
        memAllocInfo.allocationSize = memReqs.size;
        // Get memory type that can be mapped to host memory
        memAllocInfo.memoryTypeIndex = _vulkan_device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        checkResult(vkAllocateMemory(_device, &memAllocInfo, nullptr, &mappableMemory));
        checkResult(vkBindImageMemory(_device, mappableImage, mappableMemory, 0));

        // Map image memory
        void *data;
        checkResult(vkMapMemory(_device, mappableMemory, 0, memReqs.size, 0, &data));
        // Copy image data of the first mip level into memory
        memcpy(data, tex->bytes()->buf(), tex->bytes()->size());
        vkUnmapMemory(_device, mappableMemory);

        // Linear tiled images don't need to be staged and can be directly used as textures
        texture.image = mappableImage;
        texture.deviceMemory = mappableMemory;
        texture.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // Setup image memory barrier transfer image to shader read layout
        VkCommandBuffer copyCmd = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

        // The sub resource range describes the regions of the image we will be transition
        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.layerCount = 1;

        // Transition the texture image layout to shader read, so it can be sampled from
        VkImageMemoryBarrier imageMemoryBarrier = vks::initializers::imageMemoryBarrier();;
        imageMemoryBarrier.image = texture.image;
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

        flushCommandBuffer(copyCmd, _queue, true);
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
    sampler.maxLod = (useStaging) ? (float)texture.mipLevels : 0.0f;
    // Enable anisotropic filtering
    // This feature is optional, so we must check if it's supported on the device
    if (_vulkan_device->features.samplerAnisotropy) {
        // Use max. level of anisotropy for this example
        sampler.maxAnisotropy = _vulkan_device->properties.limits.maxSamplerAnisotropy;
        sampler.anisotropyEnable = VK_TRUE;
    } else {
        // The device does not support anisotropic filtering
        sampler.maxAnisotropy = 1.0;
        sampler.anisotropyEnable = VK_FALSE;
    }
    sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    checkResult(vkCreateSampler(_device, &sampler, nullptr, &texture.sampler));

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
    view.subresourceRange.levelCount = (useStaging) ? texture.mipLevels : 1;
    // The view will be based on the texture's image
    view.image = texture.image;
    checkResult(vkCreateImageView(_device, &view, nullptr, &texture.view));
}

VkCommandBuffer VulkanAPI::createCommandBuffer(VkCommandBufferLevel level, bool begin) const
{
    VkCommandBuffer cmdBuffer;

    VkCommandBufferAllocateInfo cmdBufAllocateInfo =
            vks::initializers::commandBufferAllocateInfo(
                _command_pool,
                level,
                1);

    checkResult(vkAllocateCommandBuffers(_device, &cmdBufAllocateInfo, &cmdBuffer));

    // If requested, also start the new command buffer
    if (begin)
    {
        VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
        checkResult(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));
    }

    return cmdBuffer;
}

void VulkanAPI::flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free) const
{
    if (commandBuffer == VK_NULL_HANDLE)
    {
        return;
    }

    checkResult(vkEndCommandBuffer(commandBuffer));

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    checkResult(vkQueueSubmit(_queue, 1, &submitInfo, VK_NULL_HANDLE));
    checkResult(vkQueueWaitIdle(_queue));

    if (free)
    {
        vkFreeCommandBuffers(_device, _command_pool, 1, &commandBuffer);
    }
}

void VulkanAPI::destroyTextureImage(VulkanAPI::Texture texture) const
{
    vkDestroyImageView(_device, texture.view, nullptr);
    vkDestroyImage(_device, texture.image, nullptr);
    vkDestroySampler(_device, texture.sampler, nullptr);
    vkFreeMemory(_device, texture.deviceMemory, nullptr);
}

}
}

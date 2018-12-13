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
#include "renderer/base/resource_manager.h"

#include "renderer/vulkan/base/vk_buffer.h"
#include "renderer/vulkan/base/vk_command_pool.h"
#include "renderer/vulkan/base/vk_device.h"
#include "renderer/vulkan/base/vk_instance.h"
#include "renderer/vulkan/base/vk_pipeline.h"
#include "renderer/vulkan/base/vk_render_target.h"
#include "renderer/vulkan/base/vk_texture.h"
#include "renderer/vulkan/pipeline_factory/pipeline_factory_vulkan.h"
#include "renderer/vulkan/util/vulkan_tools.h"
#include "renderer/vulkan/util/vulkan_debug.h"

#include "generated/vulkan_plugin.h"

#define VERTEX_BUFFER_BIND_ID 0

namespace ark {

#ifdef _WIN32
    extern HINSTANCE gInstance;
    extern HWND gWnd;
#endif

namespace vulkan {

VulkanAPI::VulkanAPI(const sp<ResourceManager>& resourceManager)
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
        vkDestroyShaderModule(_device->logicalDevice(), shaderModule, nullptr);
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
    _instance = sp<VKInstance>::make();
    _instance->initialize();
    _device = sp<VKDevice>::make(_instance, _instance->physicalDevices()[0]);
    _render_target = sp<VKRenderTarget>::make(_device);

    _uniforms = sp<VKBuffer>::make(_device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(uboVS));

    updateUniformBuffers();
    generateQuad();

    _pipeline_factory = sp<PipelineFactoryVulkan>::make(_resource_manager);
    _pipeline_factory->_texture = sp<VKTexture>::make(_resource_manager, _render_target->commandPool());
    _pipeline_factory->_buffer = _uniforms;
    _pipeline = _pipeline_factory->build(_render_target);

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

VkPipelineShaderStageCreateInfo VulkanAPI::loadShaderSPIR(VkDevice device, std::string fileName, VkShaderStageFlagBits stage)
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

}
}

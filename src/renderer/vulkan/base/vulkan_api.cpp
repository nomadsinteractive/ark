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
#include "renderer/inf/uploader.h"

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

VulkanAPI::VulkanAPI(const sp<ResourceManager>& resourceManager, const sp<RendererFactoryVulkan::Stub>& rendererFactory)
    : _ubo(sp<UBO>::make()), _resource_manager(resourceManager), _renderer_factory(rendererFactory), _graphics_context(nullptr, resourceManager)
{
}

VulkanAPI::~VulkanAPI()
{
    _renderer_factory->_device->waitIdle();

    vertexBuffer.destroy();
    indexBuffer.destroy();

    _renderer_factory->_render_target->commandPool()->destroyCommandBuffers(_command_buffers.size(), _command_buffers.data());

    for (auto& shaderModule : shaderModules)
    {
        vkDestroyShaderModule(_renderer_factory->_device->logicalDevice(), shaderModule, nullptr);
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

    _uniforms = sp<VKBuffer>::make(_renderer_factory->_device, _resource_manager->recycler(), sp<Uploader::StandardLayout<UBO>>::make(_ubo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    updateUniformBuffers(_renderer_factory->_render_target);
    generateQuad(_renderer_factory->_device);

    _pipeline_factory = sp<PipelineFactoryVulkan>::make(_resource_manager, _renderer_factory->_render_target);
    _pipeline_factory->_texture = sp<VKTexture>::make(_resource_manager->recycler(), _resource_manager, _renderer_factory->_render_target->commandPool());
    _pipeline_factory->_buffer = _uniforms;
    _pipeline = _pipeline_factory->build();

    buildCommandBuffers(_renderer_factory->_render_target);
    prepared = true;
}

void VulkanAPI::render()
{
    if (!prepared)
        return;
    draw(_renderer_factory->_render_target);
}

void VulkanAPI::updateUniformBuffers(const VKRenderTarget& renderTarget)
{
    _ubo->projection = glm::perspective(glm::radians(60.0f), (float)renderTarget.width() / (float)renderTarget.height(), 0.001f, 256.0f);
    glm::mat4 viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, zoom));

    _ubo->model = viewMatrix * glm::translate(glm::mat4(1.0f), cameraPos);
    _ubo->model = glm::rotate(_ubo->model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    _ubo->model = glm::rotate(_ubo->model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    _ubo->model = glm::rotate(_ubo->model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    _ubo->viewPos = glm::vec4(0.0f, 0.0f, -zoom, 0.0f);

    _uniforms->upload(_graphics_context);
}

void VulkanAPI::buildCommandBuffers(const VKRenderTarget& renderTarget)
{
    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

    VkClearValue clearValues[2];
    clearValues[0].color = defaultClearColor;
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
    renderPassBeginInfo.renderPass = renderTarget.renderPass();
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = renderTarget.width();
    renderPassBeginInfo.renderArea.extent.height = renderTarget.height();
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;

    _command_buffers = renderTarget.makeCommandBuffers();
    for (size_t i = 0; i < _command_buffers.size(); ++i)
    {
        // Set target frame buffer
        renderPassBeginInfo.framebuffer = renderTarget.frameBuffers()[i];

        checkResult(vkBeginCommandBuffer(_command_buffers[i], &cmdBufInfo));

        vkCmdBeginRenderPass(_command_buffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = vks::initializers::viewport((float)renderTarget.width(), (float)renderTarget.height(), 0.0f, 1.0f);
        vkCmdSetViewport(_command_buffers[i], 0, 1, &viewport);

        VkRect2D scissor = vks::initializers::rect2D(renderTarget.width(), renderTarget.height(), 0, 0);
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

void VulkanAPI::generateQuad(const VKDevice& device)
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
    checkResult(device.createBuffer(
                        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        &vertexBuffer,
                        vertices.size() * sizeof(Vertex),
                        vertices.data()));
    // Index buffer
    checkResult(device.createBuffer(
                        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        &indexBuffer,
                        indices.size() * sizeof(uint32_t),
                        indices.data()));
}

void VulkanAPI::draw(VKRenderTarget& renderTarget)
{
    uint32_t currentBuffer = renderTarget.acquire();
    renderTarget.submit(&_command_buffers[currentBuffer]);
    renderTarget.swap(currentBuffer);
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

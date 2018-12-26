#include "renderer/vulkan/renderer_factory/renderer_factory_vulkan.h"

#include "core/base/plugin_manager.h"
#include "core/types/global.h"
#include "core/util/log.h"

#include "graphics/base/size.h"

#include "renderer/base/gl_context.h"
#include "renderer/base/resource_manager.h"

#include "renderer/vulkan/base/vk_instance.h"
#include "renderer/vulkan/base/vk_device.h"
#include "renderer/vulkan/base/vk_render_target.h"
#include "renderer/vulkan/render_view/render_view_vulkan.h"
#include "renderer/vulkan/snippet_factory/snippet_factory_vulkan.h"
#include "renderer/vulkan/pipeline_factory/pipeline_factory_vulkan.h"

#include "renderer/vulkan/base/vk_util.h"
#include "renderer/vulkan/base/vk_buffer.h"
#include "renderer/vulkan/base/vk_renderer.h"
#include "renderer/vulkan/base/vk_texture_2d.h"

#include "generated/vulkan_plugin.h"

namespace ark {
namespace vulkan {

RendererFactoryVulkan::RendererFactoryVulkan(const sp<ResourceManager>& resourceManager)
    : _resource_manager(resourceManager), _renderer(sp<VKRenderer>::make())
{
    const Global<PluginManager> pm;
    pm->addPlugin(sp<VulkanPlugin>::make());
}

RendererFactoryVulkan::~RendererFactoryVulkan()
{
}

void RendererFactoryVulkan::initialize(GLContext& glContext)
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);

    setGLVersion(Ark::VULKAN_11, glContext);

    _renderer->_instance = sp<VKInstance>::make();
    _renderer->_instance->initialize();

    _renderer->_device = sp<VKDevice>::make(_renderer->_instance, _renderer->_instance->physicalDevices()[0]);
    _renderer->_render_target = sp<VKRenderTarget>::make(_renderer->_device);
}

void RendererFactoryVulkan::setGLVersion(Ark::RendererVersion version, GLContext& glContext)
{
    DCHECK(version != Ark::AUTO, "Cannot set Vulkan version to \"auto\" manually.");
    LOGD("Choose GLVersion = %d", version);
    std::map<String, String>& annotations = glContext.annotations();

    annotations["vert.in"] = "in";
    annotations["vert.out"] = "out";
    annotations["frag.in"] = "in";
    annotations["frag.out"] = "out";
    annotations["frag.color"] = "v_FragColor";
    glContext.setSnippetFactory(sp<SnippetFactoryVulkan>::make());

    glContext.setVersion(version);
}

sp<Buffer::Delegate> RendererFactoryVulkan::createBuffer(Buffer::Type type, Buffer::Usage /*usage*/, const sp<Uploader>& uploader)
{
    static const VkBufferUsageFlags usagesFlags[Buffer::TYPE_COUNT] = {VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT};
    return sp<VKBuffer>::make(_renderer, _resource_manager->recycler(), uploader, usagesFlags[type], VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

sp<RenderView> RendererFactoryVulkan::createRenderView(const sp<GLContext>& glContext, const Viewport& viewport)
{
    return sp<RenderViewVulkan>::make(sp<VKUtil>::make(_resource_manager, _renderer), glContext, _resource_manager, viewport);
}

sp<PipelineFactory> RendererFactoryVulkan::createPipelineFactory()
{
    return sp<PipelineFactoryVulkan>::make(_resource_manager, _renderer);
}

sp<Texture> RendererFactoryVulkan::createTexture(const sp<Recycler>& recycler, uint32_t width, uint32_t height, const sp<Variable<bitmap>>& bitmap)
{
    const sp<Size> size = sp<Size>::make(width, height);
    const sp<VKTexture2D> texture = sp<VKTexture2D>::make(recycler, _renderer, bitmap);
    return sp<Texture>::make(size, texture, Texture::TYPE_2D);
}

}
}

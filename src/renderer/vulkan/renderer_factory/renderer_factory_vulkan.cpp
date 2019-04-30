#include "renderer/vulkan/renderer_factory/renderer_factory_vulkan.h"

#include "core/base/plugin_manager.h"
#include "core/types/global.h"
#include "core/util/log.h"

#include "graphics/base/camera.h"
#include "graphics/base/size.h"
#include "graphics/base/viewport.h"

#include "renderer/base/framebuffer.h"
#include "renderer/base/render_context.h"
#include "renderer/base/render_controller.h"

#include "renderer/vulkan/base/vk_instance.h"
#include "renderer/vulkan/base/vk_device.h"
#include "renderer/vulkan/base/vk_render_target.h"
#include "renderer/vulkan/render_view/render_view_vulkan.h"
#include "renderer/vulkan/snippet_factory/snippet_factory_vulkan.h"
#include "renderer/vulkan/pipeline_factory/pipeline_factory_vulkan.h"

#include "renderer/vulkan/base/vk_buffer.h"
#include "renderer/vulkan/base/vk_framebuffer.h"
#include "renderer/vulkan/base/vk_framebuffer_renderer.h"
#include "renderer/vulkan/base/vk_heap.h"
#include "renderer/vulkan/base/vk_renderer.h"
#include "renderer/vulkan/base/vk_texture_2d.h"
#include "renderer/vulkan/util/vk_util.h"

#include "platform/platform.h"

#include "generated/vulkan_plugin.h"


namespace ark {
namespace vulkan {

RendererFactoryVulkan::RendererFactoryVulkan(const sp<Recycler>& recycler)
    : _recycler(recycler), _renderer(sp<VKRenderer>::make())
{
    const Global<PluginManager> pm;
    pm->addPlugin(sp<VulkanPlugin>::make());
}

RendererFactoryVulkan::~RendererFactoryVulkan()
{
}

sp<RenderContext> RendererFactoryVulkan::initialize(Ark::RendererVersion version)
{
    const sp<RenderContext> vkContext = sp<RenderContext>::make(version, Viewport(0, 0.0f, 1.0f, 1.0f, 0, 1.0f), -1.0f);
    if(version != Ark::AUTO)
        setVersion(version, vkContext);
    return vkContext;
}

void RendererFactoryVulkan::onSurfaceCreated(RenderContext& vkContext)
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);

    setVersion(Ark::VULKAN_11, vkContext);

    Platform::vkInitialize();

    _renderer->_instance = sp<VKInstance>::make();
    _renderer->_instance->initialize();

    _renderer->_device = sp<VKDevice>::make(_renderer->_instance, _renderer->_instance->physicalDevices()[0]);
    _renderer->_heap = sp<VKHeap>::make(_renderer->_device);
    _renderer->_render_target = sp<VKRenderTarget>::make(vkContext, _renderer->_device);
}

void RendererFactoryVulkan::setVersion(Ark::RendererVersion version, RenderContext& vkContext)
{
    LOGD("Choose Vulkan Version = %d", version);
    std::map<String, String>& annotations = vkContext.annotations();

    annotations["vert.in"] = "in";
    annotations["vert.out"] = "out";
    annotations["frag.in"] = "in";
    annotations["frag.out"] = "out";
    annotations["frag.color"] = "v_FragColor";
    vkContext.setSnippetFactory(sp<SnippetFactoryVulkan>::make());

    vkContext.setVersion(version);
}

sp<Buffer::Delegate> RendererFactoryVulkan::createBuffer(Buffer::Type type, Buffer::Usage /*usage*/)
{
    static const VkBufferUsageFlags usagesFlags[Buffer::TYPE_COUNT] = {VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT};
    return sp<VKBuffer>::make(_renderer, _recycler, usagesFlags[type], VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

sp<Framebuffer> RendererFactoryVulkan::createFramebuffer(const sp<Renderer>& renderer, const sp<Texture>& texture)
{
    const sp<VKFramebuffer> fbo = sp<VKFramebuffer>::make(_renderer, _recycler, texture);
    return sp<Framebuffer>::make(fbo, sp<VKFramebufferRenderer>::make(renderer, fbo));
}

sp<RenderView> RendererFactoryVulkan::createRenderView(const sp<RenderContext>& renderContext, const sp<RenderController>& renderController, const Viewport& viewport)
{
    return sp<RenderViewVulkan>::make(_renderer, renderContext, renderController, viewport);
}

sp<PipelineFactory> RendererFactoryVulkan::createPipelineFactory()
{
    return sp<PipelineFactoryVulkan>::make(_recycler, _renderer);
}

sp<Texture> RendererFactoryVulkan::createTexture(uint32_t width, uint32_t height, const sp<Texture::Uploader>& uploader)
{
    const sp<Size> size = sp<Size>::make(static_cast<float>(width), static_cast<float>(height));
    const sp<VKTexture2D> texture = sp<VKTexture2D>::make(_recycler, _renderer, width, height, sp<Texture::Parameters>::make(), uploader);
    return sp<Texture>::make(size, sp<Variable<sp<Texture::Delegate>>::Const>::make(texture), Texture::TYPE_2D);
}

}
}

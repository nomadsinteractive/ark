#include "renderer/vulkan/renderer_factory/renderer_factory_vulkan.h"

#include "core/base/plugin_manager.h"
#include "core/types/global.h"
#include "core/util/log.h"

#include "graphics/base/camera.h"
#include "graphics/base/size.h"
#include "graphics/base/viewport.h"

#include "renderer/base/framebuffer.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/base/render_controller.h"

#include "renderer/impl/snippet_factory/snippet_factory_vulkan.h"
#include "renderer/vulkan/base/vk_instance.h"
#include "renderer/vulkan/base/vk_device.h"
#include "renderer/vulkan/base/vk_swap_chain.h"
#include "renderer/vulkan/render_view/render_view_vulkan.h"
#include "renderer/vulkan/pipeline_factory/pipeline_factory_vulkan.h"

#include "renderer/vulkan/base/vk_buffer.h"
#include "renderer/vulkan/base/vk_framebuffer.h"
#include "renderer/vulkan/base/vk_framebuffer_renderer.h"
#include "renderer/vulkan/base/vk_heap.h"
#include "renderer/vulkan/base/vk_renderer.h"
#include "renderer/vulkan/base/vk_texture.h"
#include "renderer/vulkan/util/vk_util.h"

#include "platform/platform.h"

#include "generated/vulkan_plugin.h"
#include "renderer/base/render_engine.h"


namespace ark::vulkan {

namespace {

void setVersion(Ark::RendererVersion version, RenderEngineContext& vkContext)
{
    LOGD("Choose Vulkan Version = %d", version);
    std::map<String, String>& definitions = vkContext.definitions();

    definitions["vert.in"] = "in";
    definitions["vert.out"] = "out";
    definitions["frag.in"] = "in";
    definitions["frag.out"] = "out";
    definitions["frag.color"] = "f_FragColor";
    vkContext.setSnippetFactory(sp<SnippetFactoryVulkan>::make());

    vkContext.setVersion(version);
}

}

RendererFactoryVulkan::RendererFactoryVulkan(sp<Recycler> recycler)
    : RendererFactory({Ark::COORDINATE_SYSTEM_LHS, true, sizeof(float)}), _recycler(std::move(recycler)), _renderer(sp<VKRenderer>::make())
{
    const Global<PluginManager> pm;
    pm->addPlugin(sp<VulkanPlugin>::make());
}

sp<RenderEngineContext> RendererFactoryVulkan::createRenderEngineContext(const ApplicationManifest::Renderer& renderer)
{
    sp<RenderEngineContext> vkContext = sp<RenderEngineContext>::make(renderer, Viewport(0, 0.0f, 1.0f, 1.0f, 0, 1.0f));
    if(renderer._version != Ark::RENDERER_VERSION_AUTO)
        setVersion(renderer._version, vkContext);
    return vkContext;
}

void RendererFactoryVulkan::onSurfaceCreated(RenderEngine& renderEngine)
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);

    setVersion(Ark::RENDERER_VERSION_VULKAN_12, renderEngine.context());

    Platform::vkInitialize();

    _renderer->_instance = sp<VKInstance>::make();
    _renderer->_instance->initialize(renderEngine);

    _renderer->_device = sp<VKDevice>::make(_renderer->_instance, _renderer->_instance->physicalDevices()[0]);
    _renderer->_heap = sp<VKHeap>::make(_renderer->_device);
    _renderer->_render_target = sp<VKSwapChain>::make(renderEngine, _renderer->_device);
}

sp<Buffer::Delegate> RendererFactoryVulkan::createBuffer(Buffer::Type type, Buffer::Usage usage)
{
    constexpr VkBufferUsageFlags usagesFlags[Buffer::TYPE_COUNT] = {VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT};
    return sp<VKBuffer>::make(_renderer, _recycler, usagesFlags[type], usage == Buffer::USAGE_STATIC ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
                                                                                                     : VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

sp<Camera::Delegate> RendererFactoryVulkan::createCamera()
{
    return sp<Camera::Delegate>::make<Camera::DelegateLH_ZO>();
}

sp<Framebuffer> RendererFactoryVulkan::createFramebuffer(sp<Renderer> renderer, std::vector<sp<Texture>> colorAttachments, sp<Texture> depthStencilAttachments, int32_t clearMask)
{
    sp<VKFramebuffer> fbo = sp<VKFramebuffer>::make(_renderer, _recycler, std::move(colorAttachments), std::move(depthStencilAttachments), clearMask);
    return sp<Framebuffer>::make(sp<VKFramebufferRenderer>::make(std::move(renderer), fbo), std::move(fbo));
}

sp<RenderView> RendererFactoryVulkan::createRenderView(const sp<RenderEngineContext>& renderContext, const sp<RenderController>& renderController)
{
    return sp<RenderViewVulkan>::make(_renderer, renderContext, renderController);
}

sp<PipelineFactory> RendererFactoryVulkan::createPipelineFactory()
{
    return sp<PipelineFactoryVulkan>::make(_recycler, _renderer);
}

sp<Texture::Delegate> RendererFactoryVulkan::createTexture(sp<Size> size, sp<Texture::Parameters> parameters)
{
    if(parameters->_type == Texture::TYPE_2D || parameters->_type == Texture::TYPE_CUBEMAP)
        return sp<VKTexture>::make(_recycler, _renderer, static_cast<uint32_t>(size->widthAsFloat()), static_cast<uint32_t>(size->heightAsFloat()), std::move(parameters));
    return nullptr;
}

}

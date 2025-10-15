#include "vulkan/impl/renderer_factory/renderer_factory_vulkan.h"

#include "core/base/plugin_manager.h"
#include "core/types/global.h"
#include "core/util/log.h"

#include "graphics/base/camera.h"
#include "graphics/components/size.h"
#include "graphics/base/viewport.h"

#include "renderer/base/render_target.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/base/render_controller.h"

#include "vulkan/base/vk_instance.h"
#include "vulkan/base/vk_device.h"
#include "vulkan/base/vk_swap_chain.h"
#include "vulkan/impl/render_view/render_view_vulkan.h"
#include "vulkan/impl/snippet_factory/snippet_factory_vulkan.h"
#include "vulkan/impl/pipeline_factory/pipeline_factory_vulkan.h"

#include "vulkan/base/vk_buffer.h"
#include "vulkan/base/vk_framebuffer.h"
#include "vulkan/base/vk_framebuffer_renderer.h"
#include "vulkan/base/vk_heap.h"
#include "vulkan/base/vk_renderer.h"
#include "vulkan/base/vk_texture.h"
#include "vulkan/util/vk_util.h"

#include "platform/platform.h"

#include "generated/vulkan_plugin.h"
#include "renderer/base/render_engine.h"


namespace ark::plugin::vulkan {

namespace {

void setVersion(const enums::RendererVersion version, RenderEngineContext& vkContext)
{
    LOGD("Choose Vulkan Version = %d", version);
    vkContext.setSnippetFactory(sp<SnippetFactory>::make<SnippetFactoryVulkan>());
    vkContext.setVersion(version);
}

}

RendererFactoryVulkan::RendererFactoryVulkan()
    : RendererFactory({{enums::RENDERING_BACKEND_BIT_VULKAN}, enums::COORDINATE_SYSTEM_LHS, true, sizeof(float)}), _renderer(sp<VKRenderer>::make())
{
}

sp<RenderEngineContext> RendererFactoryVulkan::createRenderEngineContext(const ApplicationManifest::Renderer& renderer)
{
    sp<RenderEngineContext> vkContext = sp<RenderEngineContext>::make(renderer, Viewport(0, 0.0f, 1.0f, 1.0f, 0, 1.0f));
    if(renderer._version != enums::RENDERER_VERSION_AUTO)
        setVersion(renderer._version, vkContext);
    return vkContext;
}

void RendererFactoryVulkan::onSurfaceCreated(RenderEngine& renderEngine)
{
    DTHREAD_CHECK(THREAD_NAME_ID_RENDERER);

    setVersion(enums::RENDERER_VERSION_VULKAN_12, renderEngine.context());

    _renderer->_instance = sp<VKInstance>::make();
    _renderer->_instance->initialize(renderEngine);

    _renderer->_device = sp<VKDevice>::make(_renderer->_instance, _renderer->_instance->physicalDevices()[0], renderEngine.context()->version());
    _renderer->_heap = sp<VKHeap>::make(_renderer->_device);
    _renderer->_render_target = sp<VKSwapChain>::make(renderEngine, _renderer->_device);
}

sp<Buffer::Delegate> RendererFactoryVulkan::createBuffer(const Buffer::Usage usage)
{
    VkBufferUsageFlags usageFlags = 0;
    constexpr VkBufferUsageFlags usageFlagsFromType[] = {VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT};
    for(uint32_t i = Buffer::USAGE_BIT_VERTEX; i <= Buffer::USAGE_BIT_STORAGE; ++i)
        if(usage.contains(static_cast<Buffer::UsageBit>(i)))
            usageFlags |= usageFlagsFromType[i];

    VkMemoryPropertyFlags flags = 0;
    if(usage.contains(Buffer::USAGE_BIT_TRANSFER_SRC))
        usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if(usage.contains(Buffer::USAGE_BIT_HOST_VISIBLE))
        flags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    else
        flags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    return sp<VKBuffer>::make(_renderer, Ark::instance().renderController()->recycler(), usageFlags, flags);
}

sp<Camera::Delegate> RendererFactoryVulkan::createCamera(const enums::CoordinateSystem rcs)
{
    return rcs == enums::COORDINATE_SYSTEM_RHS ? sp<Camera::Delegate>::make<Camera::DelegateRH_ZO>() : sp<Camera::Delegate>::make<Camera::DelegateLH_ZO>();
}

sp<RenderTarget> RendererFactoryVulkan::createRenderTarget(sp<Renderer> renderer, RenderTarget::Configure configure)
{
    if(const sp<RenderLayer> renderLayer = renderer.asInstance<RenderLayer>())
        if(const auto& traits = renderLayer->shader()->pipelineDesciptor()->configuration()._traits; traits.has(PipelineDescriptor::TRAIT_TYPE_DEPTH_TEST))
            configure._depth_test_write_enabled = traits.at(PipelineDescriptor::TRAIT_TYPE_DEPTH_TEST)._depth_test._write_enabled;

    sp<VKFramebuffer> fbo = sp<VKFramebuffer>::make(_renderer, Ark::instance().renderController()->recycler(), std::move(configure));
    sp<Renderer> fboRenderer = sp<Renderer>::make<VKFramebufferRenderer>(renderer, fbo);
    return sp<RenderTarget>::make(std::move(renderer), std::move(fbo), std::move(fboRenderer));
}

sp<RenderView> RendererFactoryVulkan::createRenderView(const sp<RenderEngineContext>& renderContext, const sp<RenderController>& renderController)
{
    return sp<RenderView>::make<RenderViewVulkan>(_renderer, renderContext, renderController);
}

sp<PipelineFactory> RendererFactoryVulkan::createPipelineFactory()
{
    return sp<PipelineFactory>::make<PipelineFactoryVulkan>(Ark::instance().renderController()->recycler(), _renderer);
}

sp<Texture::Delegate> RendererFactoryVulkan::createTexture(sp<Size> size, sp<Texture::Parameters> parameters)
{
    if(parameters->_type == Texture::TYPE_2D || parameters->_type == Texture::TYPE_CUBEMAP)
        return sp<VKTexture>::make(Ark::instance().renderController()->recycler(), _renderer, static_cast<uint32_t>(size->widthAsFloat()), static_cast<uint32_t>(size->heightAsFloat()), std::move(parameters));
    return nullptr;
}

sp<RendererFactory> RendererFactoryVulkan::BUILDER::build(const Scope& /*args*/)
{
    return sp<RendererFactory>::make<RendererFactoryVulkan>();
}

}

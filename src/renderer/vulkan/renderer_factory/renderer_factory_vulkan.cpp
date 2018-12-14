#include "renderer/vulkan/renderer_factory/renderer_factory_vulkan.h"

#include "core/base/plugin_manager.h"
#include "core/types/global.h"
#include "core/util/log.h"

#include "renderer/base/gl_context.h"

#include "renderer/vulkan/base/vk_instance.h"
#include "renderer/vulkan/base/vk_device.h"
#include "renderer/vulkan/base/vk_render_target.h"
#include "renderer/vulkan/render_view/render_view_vulkan.h"
#include "renderer/vulkan/snippet_factory/snippet_factory_vulkan.h"
#include "renderer/vulkan/pipeline_factory/pipeline_factory_vulkan.h"

#include "renderer/vulkan/base/vulkan_api.h"

#include "generated/vulkan_plugin.h"

namespace ark {
namespace vulkan {

RendererFactoryVulkan::RendererFactoryVulkan(const sp<ResourceManager>& resourceManager)
    : _resource_manager(resourceManager), _stub(sp<Stub>::make())
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

    _stub->_instance = sp<VKInstance>::make();
    _stub->_instance->initialize();

    _stub->_device = sp<VKDevice>::make(_stub->_instance, _stub->_instance->physicalDevices()[0]);
    _stub->_render_target = sp<VKRenderTarget>::make(_stub->_device);
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
    glContext.setGLSnippetFactory(sp<SnippetFactoryVulkan>::make());

    glContext.setVersion(version);
}

sp<Buffer::Delegate> RendererFactoryVulkan::createBuffer(Buffer::Type type, Buffer::Usage usage, const sp<Uploader>& uploader)
{
    return nullptr;
}

sp<RenderView> RendererFactoryVulkan::createRenderView(const sp<GLContext>& glContext, const Viewport& viewport)
{
    return sp<RenderViewVulkan>::make(sp<VulkanAPI>::make(_resource_manager, _stub), glContext, _resource_manager, viewport);
}

sp<PipelineFactory> RendererFactoryVulkan::createPipelineFactory()
{
    return sp<PipelineFactoryVulkan>::make(_resource_manager, _stub->_render_target);
}

sp<Texture> RendererFactoryVulkan::createTexture(const sp<Recycler>& recycler, uint32_t width, uint32_t height, const sp<Variable<bitmap>>& bitmap)
{
    return nullptr;
}

}
}

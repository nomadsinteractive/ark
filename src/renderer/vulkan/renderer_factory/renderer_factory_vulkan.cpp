#include "renderer/vulkan/renderer_factory/renderer_factory_vulkan.h"

#include "core/base/plugin_manager.h"
#include "core/types/global.h"
#include "core/util/log.h"

#include "renderer/base/gl_context.h"

#include "renderer/vulkan/render_view/render_view_vulkan.h"
#include "renderer/vulkan/gl_snippet_factory/gl_snippet_factory_vulkan.h"
#include "renderer/vulkan/pipeline_factory/pipeline_factory_vulkan.h"

#include "generated/vulkan_plugin.h"

namespace ark {
namespace vulkan {

RendererFactoryVulkan::RendererFactoryVulkan(const sp<ResourceManager>& glResources)
    : _resource_manager(glResources), _vulkan_api(sp<VulkanAPI>::make(glResources))
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
    glContext.setGLSnippetFactory(sp<GLSnippetFactoryVulkan>::make());

    glContext.setVersion(version);
}

sp<Buffer::Delegate> RendererFactoryVulkan::createBuffer(Buffer::Type type, Buffer::Usage usage, const sp<Buffer::Uploader>& uploader)
{
    return nullptr;
}

sp<RenderView> RendererFactoryVulkan::createRenderView(const sp<GLContext>& glContext, const Viewport& viewport)
{
    return sp<RenderViewVulkan>::make(_vulkan_api, glContext, _resource_manager, viewport);
}

sp<ark::PipelineFactory> RendererFactoryVulkan::createPipelineFactory()
{
    return sp<PipelineFactoryVulkan>::make(_resource_manager);
}

}
}

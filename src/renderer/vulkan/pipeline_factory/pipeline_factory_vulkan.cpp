#include "renderer/vulkan/pipeline_factory/pipeline_factory_vulkan.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/shader_bindings.h"

#include "renderer/vulkan/base/vk_pipeline.h"

namespace ark {
namespace vulkan {

PipelineFactoryVulkan::PipelineFactoryVulkan(const sp<Recycler>& recycler, const sp<VKRenderer>& renderFactory)
    : _recycler(recycler), _renderer(renderFactory)
{
}

sp<Pipeline> PipelineFactoryVulkan::buildPipeline(GraphicsContext& graphicsContext, const sp<ShaderBindings>& shaderBindings)
{
    std::map<Shader::Stage, String> shaders = shaderBindings->pipelineLayout()->getPreprocessedShaders(graphicsContext.renderContext());
    return sp<VKPipeline>::make(_recycler, _renderer, shaderBindings, std::move(shaders));
}

}
}

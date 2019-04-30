#include "renderer/vulkan/pipeline_factory/pipeline_factory_vulkan.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/render_controller.h"

#include "renderer/vulkan/base/vk_pipeline.h"

namespace ark {
namespace vulkan {

PipelineFactoryVulkan::PipelineFactoryVulkan(const sp<Recycler>& recycler, const sp<VKRenderer>& renderFactory)
    : _recycler(recycler), _renderer(renderFactory)
{
}

sp<Pipeline> PipelineFactoryVulkan::buildPipeline(GraphicsContext& graphicsContext, const PipelineBindings& bindings)
{
    std::map<Shader::Stage, String> shaders = bindings.layout()->getPreprocessedShaders(graphicsContext.renderContext());
    return sp<VKPipeline>::make(bindings, _recycler, _renderer, std::move(shaders));
}

}
}

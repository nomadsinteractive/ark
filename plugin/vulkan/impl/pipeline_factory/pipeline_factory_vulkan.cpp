#include "vulkan/impl/pipeline_factory/pipeline_factory_vulkan.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/pipeline_configuration.h"
#include "renderer/base/pipeline_descriptor.h"

#include "vulkan/base/vk_pipeline.h"

namespace ark::plugin::vulkan {

PipelineFactoryVulkan::PipelineFactoryVulkan(const sp<Recycler>& recycler, const sp<VKRenderer>& renderFactory)
    : _recycler(recycler), _renderer(renderFactory)
{
}

sp<Pipeline> PipelineFactoryVulkan::buildPipeline(GraphicsContext& graphicsContext, const PipelineBindings& pipelineBindings, Map<Enum::ShaderStageBit, String> stages)
{
    return sp<Pipeline>::make<VKPipeline>(pipelineBindings, _recycler, _renderer, std::move(stages));
}

}

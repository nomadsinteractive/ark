#include "renderer/vulkan/pipeline_factory/pipeline_factory_vulkan.h"

#include <array>

#include "renderer/base/drawing_context.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/resource_manager.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/ubo.h"
#include "renderer/inf/uploader.h"

#include "renderer/vulkan/base/vk_buffer.h"
#include "renderer/vulkan/base/vk_device.h"
#include "renderer/vulkan/base/vk_pipeline.h"
#include "renderer/vulkan/base/vk_renderer.h"
#include "renderer/vulkan/base/vk_render_target.h"
#include "renderer/vulkan/base/vk_texture_2d.h"
#include "renderer/vulkan/base/vk_util.h"

namespace ark {
namespace vulkan {

PipelineFactoryVulkan::PipelineFactoryVulkan(const sp<ResourceManager>& resourceManager, const sp<VKRenderer>& renderFactory)
    : _resource_manager(resourceManager), _renderer(renderFactory)
{
}

sp<Pipeline> PipelineFactoryVulkan::buildPipeline(GraphicsContext& graphicsContext, const sp<ShaderBindings>& shaderBindings)
{
    return sp<VKPipeline>::make(_resource_manager->recycler(), _renderer, shaderBindings);
}

}
}

#include "renderer/vulkan/base/vk_pipeline.h"

#include "renderer/base/recycler.h"

#include "renderer/vulkan/base/vk_device.h"
#include "renderer/vulkan/base/vk_render_target.h"
#include "renderer/vulkan/util/vulkan_tools.h"

namespace ark {
namespace vulkan {

VKPipeline::VKPipeline(const sp<Recycler>& recycler, const sp<VKRenderTarget>& renderTarget, VkPipelineLayout layout, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet descriptorSet, VkPipeline pipeline)
    : _recycler(recycler), _render_target(renderTarget), _layout(layout), _descriptor_set_layout(descriptorSetLayout), _descriptor_set(descriptorSet), _pipeline(pipeline)
{
}

VKPipeline::~VKPipeline()
{
    _recycler->recycle(*this);
}

VkPipeline VKPipeline::vkPipeline() const
{
    return _pipeline;
}

VkPipelineLayout VKPipeline::vkPipelineLayout() const
{
    return _layout;
}

const VkDescriptorSet& VKPipeline::vkDescriptorSet() const
{
    return _descriptor_set;
}

uint32_t VKPipeline::id()
{
    return 0;
}

void VKPipeline::upload(GraphicsContext& /*graphicsContext*/)
{
}

Resource::RecycleFunc VKPipeline::recycle()
{
    const sp<VKDevice> device = _render_target->device();

    VkPipelineLayout layout = _layout;
    VkDescriptorSetLayout descriptorSetLayout = _descriptor_set_layout;
    VkPipeline pipeline = _pipeline;

    return [device, layout, descriptorSetLayout, pipeline](GraphicsContext&) {
        vkDestroyPipelineLayout(device->logicalDevice(), layout, nullptr);
        vkDestroyDescriptorSetLayout(device->logicalDevice(), descriptorSetLayout, nullptr);
        vkDestroyPipeline(device->logicalDevice(), pipeline, nullptr);
    };
}

void VKPipeline::active(GraphicsContext& graphicsContext, const DrawingContext& drawingContext)
{
}

void VKPipeline::bind(GraphicsContext& graphicsContext, const ShaderBindings& bindings)
{
}

}
}

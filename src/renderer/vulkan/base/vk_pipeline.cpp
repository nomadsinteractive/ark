#include "renderer/vulkan/base/vk_pipeline.h"

#include "renderer/vulkan/base/vk_device.h"
#include "renderer/vulkan/base/vk_render_target.h"
#include "renderer/vulkan/util/vulkan_tools.h"

namespace ark {
namespace vulkan {

VKPipeline::VKPipeline(const sp<VKRenderTarget>& renderTarget, VkPipelineLayout layout, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet descriptorSet, VkPipeline pipeline)
    : _render_target(renderTarget), _layout(layout), _descriptor_set_layout(descriptorSetLayout), _descriptor_set(descriptorSet), _pipeline(pipeline)
{
}

VKPipeline::~VKPipeline()
{
    vkDestroyPipelineLayout(_render_target->device()->logicalDevice(), _layout, nullptr);
    vkDestroyDescriptorSetLayout(_render_target->device()->logicalDevice(), _descriptor_set_layout, nullptr);
    vkDestroyPipeline(_render_target->device()->logicalDevice(), _pipeline, nullptr);
}

VkPipeline VKPipeline::pipeline() const
{
    return _pipeline;
}

VkPipelineLayout VKPipeline::layout() const
{
    return _layout;
}

const VkDescriptorSet& VKPipeline::descriptorSet() const
{
    return _descriptor_set;
}

}
}

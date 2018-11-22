#include "renderer/vulkan/base/pipeline.h"

#include "renderer/vulkan/base/device.h"
#include "renderer/vulkan/base/render_target.h"
#include "renderer/vulkan/util/vulkan_tools.h"

#define VERTEX_BUFFER_BIND_ID 0

namespace ark {
namespace vulkan {

Pipeline::Pipeline(const sp<RenderTarget>& renderTarget, VkPipelineLayout layout, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet descriptorSet, VkPipeline pipeline)
    : _render_target(renderTarget), _layout(layout), _descriptor_set_layout(descriptorSetLayout), _descriptor_set(descriptorSet), _pipeline(pipeline)
{
}

Pipeline::~Pipeline()
{
    vkDestroyPipelineLayout(_render_target->device()->logicalDevice(), _layout, nullptr);
    vkDestroyDescriptorSetLayout(_render_target->device()->logicalDevice(), _descriptor_set_layout, nullptr);
    vkDestroyPipeline(_render_target->device()->logicalDevice(), _pipeline, nullptr);
}

VkPipeline Pipeline::pipeline() const
{
    return _pipeline;
}

VkPipelineLayout Pipeline::layout() const
{
    return _layout;
}

const VkDescriptorSet& Pipeline::descriptorSet() const
{
    return _descriptor_set;
}

}
}

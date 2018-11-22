#ifndef ARK_RENDERER_VULKAN_BASE_PIPELINE_H_
#define ARK_RENDERER_VULKAN_BASE_PIPELINE_H_

#include <vector>

#include <vulkan/vulkan.h>

#include "core/types/shared_ptr.h"

#include "renderer/vulkan/forward.h"

namespace ark {
namespace vulkan {

class Pipeline {
public:
    Pipeline(const sp<RenderTarget>& renderTarget, VkPipelineLayout layout, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet descriptorSet, VkPipeline pipeline);
    ~Pipeline();

    VkPipeline pipeline() const;
    VkPipelineLayout layout() const;
    const VkDescriptorSet& descriptorSet() const;

private:
    sp<RenderTarget> _render_target;

    VkPipelineLayout _layout;
    VkDescriptorSetLayout _descriptor_set_layout;
    VkDescriptorSet _descriptor_set;
    VkPipeline _pipeline;
};

}
}

#endif

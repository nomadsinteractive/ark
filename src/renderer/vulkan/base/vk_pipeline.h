#ifndef ARK_RENDERER_VULKAN_BASE_PIPELINE_H_
#define ARK_RENDERER_VULKAN_BASE_PIPELINE_H_

#include <vector>

#include <vulkan/vulkan.h>

#include "core/types/shared_ptr.h"

#include "renderer/inf/pipeline.h"

#include "renderer/vulkan/forward.h"

namespace ark {
namespace vulkan {

class VKPipeline : public Pipeline {
public:
    VKPipeline(sp<Recycler> recycler, sp<VKRenderTarget> renderTarget, VkPipelineLayout vkPipelineLayout, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet descriptorSet, VkPipeline pipeline);
    ~VKPipeline() override;

    VkPipeline vkPipeline() const;
    VkPipelineLayout vkPipelineLayout() const;
    const VkDescriptorSet& vkDescriptorSet() const;

    virtual uint32_t id() override;
    virtual void upload(GraphicsContext& graphicsContext) override;
    virtual RecycleFunc recycle() override;

    virtual void active(GraphicsContext& graphicsContext, const PipelineInput& input) override;

    virtual void bind(GraphicsContext& graphicsContext, const ShaderBindings& bindings) override;
    virtual void activeTexture(Resource& texture, Texture::Type type, uint32_t name) override;

private:
    sp<Recycler> _recycler;
    sp<VKRenderTarget> _render_target;

    VkPipelineLayout _layout;
    VkDescriptorSetLayout _descriptor_set_layout;
    VkDescriptorSet _descriptor_set;
    VkPipeline _pipeline;
};

}
}

#endif

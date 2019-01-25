#ifndef ARK_RENDERER_VULKAN_BASE_PIPELINE_H_
#define ARK_RENDERER_VULKAN_BASE_PIPELINE_H_

#include <vector>

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/shader.h"
#include "renderer/inf/pipeline.h"

#include "renderer/vulkan/forward.h"

#include "platform/vulkan/vulkan.h"

namespace ark {
namespace vulkan {

class VKPipeline : public Pipeline {
public:
    VKPipeline(const sp<Recycler>& recycler, const sp<VKRenderer>& renderer, const sp<ShaderBindings>& shaderBindings, std::map<Shader::Stage, String> shaders);
    ~VKPipeline() override;

    VkPipeline vkPipeline() const;
    VkPipelineLayout vkPipelineLayout() const;
    const VkDescriptorSet& vkDescriptorSet() const;

    virtual uint64_t id() override;
    virtual void upload(GraphicsContext& graphicsContext, const sp<Uploader>& uploader) override;
    virtual RecycleFunc recycle() override;

    virtual sp<RenderCommand> active(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override;

private:
    struct VertexLayout {
        VkPipelineVertexInputStateCreateInfo inputState;
        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    };

    void setupVertexDescriptions(const PipelineInput& input, VertexLayout& vertexLayout);
    void setupDescriptorSetLayout(const PipelineInput& pipelineInput);
    void setupDescriptorSet(GraphicsContext& graphicsContext, const ShaderBindings& bindings);
    void setupPipeline(const VertexLayout& vertexLayout);

    void buildCommandBuffer(GraphicsContext& graphicsContext, const Buffer::Snapshot& vertex, const Buffer::Snapshot& index);
    void bind(GraphicsContext& graphicsContext, const DrawingContext& drawingContext, bool rebuildCommandBuffer);

    bool isDirty(const bytearray& dirtyFlags) const;

private:
    sp<Recycler> _recycler;
    sp<ShaderBindings> _shader_bindings;
    sp<VKRenderer> _renderer;
    sp<VKDescriptorPool> _descriptor_pool;

    VkPipelineLayout _layout;
    VkDescriptorSetLayout _descriptor_set_layout;
    VkDescriptorSet _descriptor_set;
    VkPipeline _pipeline;

    std::map<Shader::Stage, String> _shaders;

    std::vector<sp<VKBuffer>> _ubos;

    std::vector<sp<Observer>> _texture_observers;
};

}
}

#endif

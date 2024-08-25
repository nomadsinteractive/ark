#pragma once

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/pipeline_factory.h"
#include "renderer/vulkan/forward.h"

namespace ark::vulkan {

class PipelineFactoryVulkan final : public PipelineFactory {
public:
    PipelineFactoryVulkan(const sp<Recycler>& recycler, const sp<VKRenderer>& renderFactory);

    sp<Pipeline> buildPipeline(GraphicsContext& graphicsContext, const PipelineDescriptor& pipelineDescriptor, std::map<Enum::ShaderStageBit, String> stages) override;

private:
    sp<Recycler> _recycler;
    sp<VKRenderer> _renderer;
};

}

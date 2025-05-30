#pragma once

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/pipeline_factory.h"
#include "vulkan/forwarding.h"

namespace ark::plugin::vulkan {

class PipelineFactoryVulkan final : public PipelineFactory {
public:
    PipelineFactoryVulkan(const sp<Recycler>& recycler, const sp<VKRenderer>& renderFactory);

    sp<Pipeline> buildPipeline(GraphicsContext& graphicsContext, const PipelineBindings& pipelineBindings, std::map<enums::ShaderStageBit, String> stages) override;

private:
    sp<Recycler> _recycler;
    sp<VKRenderer> _renderer;
};

}

#pragma once

#include "renderer/inf/pipeline_factory.h"

namespace ark::plugin::bgfx {

class PipelineFactoryBgfx final : public PipelineFactory {
public:
    sp<Pipeline> buildPipeline(GraphicsContext& graphicsContext, const PipelineBindings& pipelineBindings, std::map<Enum::ShaderStageBit, String> stages) override;
};

}

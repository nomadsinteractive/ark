#pragma once

#include "renderer/inf/pipeline_factory.h"

namespace ark::plugin::sdl3 {

class PipelineFactorySDL3_GPU final : public PipelineFactory {
public:
    sp<Pipeline> buildPipeline(GraphicsContext& graphicsContext, const PipelineDescriptor& pipelineDescriptor, std::map<Enum::ShaderStageBit, String> stages) override;
};

}

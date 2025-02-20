#pragma once

#include "core/forwarding.h"

#include "renderer/inf/pipeline_factory.h"

namespace ark::plugin::opengl {

class PipelineFactoryOpenGL final : public PipelineFactory {
public:

    sp<Pipeline> buildPipeline(GraphicsContext& graphicsContext, const PipelineDescriptor& pipelineDescriptor, std::map<Enum::ShaderStageBit, String> stages) override;

};

}

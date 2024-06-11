#pragma once

#include "core/forwarding.h"

#include "renderer/base/shader.h"
#include "renderer/inf/pipeline_factory.h"

namespace ark::opengl {

class PipelineFactoryOpenGL : public PipelineFactory {
public:

    virtual sp<Pipeline> buildPipeline(GraphicsContext& graphicsContext, const PipelineBindings& bindings) override;

};

}

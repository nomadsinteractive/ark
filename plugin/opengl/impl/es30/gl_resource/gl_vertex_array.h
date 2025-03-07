#pragma once

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/inf/resource.h"

#include "opengl/base/gl_pipeline.h"

namespace ark::plugin::opengl {

class GLVertexArray final : public Resource {
public:
    GLVertexArray(const PipelineBindings& pipelineBindings, sp<GLPipeline> pipeline, sp<Buffer::Delegate> vertices);

    uint64_t id() override;
    void upload(GraphicsContext& graphicsContext) override;
    ResourceRecycleFunc recycle() override;

private:
    PipelineBindings _pipeline_bindings;

    sp<GLPipeline> _pipeline;
    sp<Buffer::Delegate> _vertex;

    uint32_t _id;
};

}

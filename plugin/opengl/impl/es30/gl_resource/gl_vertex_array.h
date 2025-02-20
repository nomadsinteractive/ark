#pragma once

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/inf/resource.h"

#include "opengl/base/gl_pipeline.h"

namespace ark::plugin::opengl {

class GLVertexArray final : public Resource {
public:
    GLVertexArray(sp<GLPipeline> pipeline, sp<Buffer::Delegate> vertices, const PipelineBindings& pipelineBindings);

    uint64_t id() override;
    void upload(GraphicsContext& graphicsContext) override;
    ResourceRecycleFunc recycle() override;

private:
    sp<GLPipeline> _pipeline;
    sp<Buffer::Delegate> _vertex;
    sp<PipelineDescriptor> _pipeline_descriptor;
    sp<std::map<uint32_t, Buffer>> _streams;

    uint32_t _id;
};

}

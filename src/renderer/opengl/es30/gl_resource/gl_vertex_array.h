#pragma once

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/inf/resource.h"

#include "renderer/opengl/base/gl_pipeline.h"

namespace ark::gles30 {

class GLVertexArray final : public Resource {
public:
    GLVertexArray(sp<opengl::GLPipeline> pipeline, sp<Buffer::Delegate> vertices, const ShaderBindings& shaderBindings);

    uint64_t id() override;
    void upload(GraphicsContext& graphicsContext) override;
    ResourceRecycleFunc recycle() override;

private:
    sp<opengl::GLPipeline> _pipeline;
    sp<Buffer::Delegate> _vertex;
    sp<PipelineDescriptor> _pipeline_descriptor;
    sp<std::map<uint32_t, Buffer>> _divisors;

    uint32_t _id;
};

}

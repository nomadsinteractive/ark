#pragma once

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/inf/resource.h"

#include "renderer/opengl/base/gl_pipeline.h"

namespace ark {
namespace gles30 {

class GLVertexArray : public Resource {
public:
    GLVertexArray(sp<opengl::GLPipeline> pipeline, sp<Buffer::Delegate> vertices, const ShaderBindings& shaderBindings);

    virtual uint64_t id() override;
    virtual void upload(GraphicsContext& graphicsContext) override;
    virtual ResourceRecycleFunc recycle() override;

private:
    sp<opengl::GLPipeline> _pipeline;
    sp<Buffer::Delegate> _vertex;
    sp<PipelineBindings> _pipeline_bindings;
    sp<std::map<uint32_t, Buffer>> _divisors;

    uint32_t _id;
};

}
}

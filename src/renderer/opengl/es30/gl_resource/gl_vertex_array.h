#ifndef ARK_RENDERER_GLES30_GL_VERTEX_ARRAY_H_
#define ARK_RENDERER_GLES30_GL_VERTEX_ARRAY_H_

#include "core/types/weak_ptr.h"

#include "renderer/base/buffer.h"
#include "renderer/inf/resource.h"

namespace ark {
namespace gles30 {

class GLVertexArray : public Resource {
public:
    GLVertexArray(const sp<PipelineFactory>& pipelineFactory, const sp<ShaderBindings>& shaderBindings);

    virtual uint32_t id() override;
    virtual void upload(GraphicsContext& graphicsContext) override;
    virtual RecycleFunc recycle() override;

private:
    uint32_t _id;

    sp<PipelineFactory> _pipeline_factory;
    WeakPtr<ShaderBindings> _shader_bindings;
};

}
}

#endif

#ifndef ARK_RENDERER_GLES30_GL_VERTEX_ARRAY_H_
#define ARK_RENDERER_GLES30_GL_VERTEX_ARRAY_H_

#include "core/types/shared_ptr.h"

#include "renderer/base/gl_buffer.h"
#include "renderer/base/gl_pipeline.h"
#include "renderer/inf/gl_resource.h"

namespace ark {
namespace gles30 {

class GLVertexArray : public GLResource {
public:
    GLVertexArray(const sp<GLShaderBindings>& shaderBindings, const GLPipeline& shader);

    virtual uint32_t id() override;
    virtual void prepare(GraphicsContext& graphicsContext) override;
    virtual void recycle(GraphicsContext& graphicsContext) override;

private:
    uint32_t _id;

    sp<GLShaderBindings> _shader_bindings;
    GLPipeline _shader;
};

}
}

#endif

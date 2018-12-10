#ifndef ARK_RENDERER_GLES30_GL_VERTEX_ARRAY_H_
#define ARK_RENDERER_GLES30_GL_VERTEX_ARRAY_H_

#include "core/types/shared_ptr.h"

#include "renderer/base/buffer.h"
#include "renderer/base/shader.h"
#include "renderer/inf/resource.h"

namespace ark {
namespace gles30 {

class GLVertexArray : public Resource {
public:
    GLVertexArray(const sp<ShaderBindings>& shaderBindings, const Shader& shader);

    virtual uint32_t id() override;
    virtual void upload(GraphicsContext& graphicsContext) override;
    virtual RecycleFunc recycle() override;

private:
    uint32_t _id;

    sp<ShaderBindings> _shader_bindings;
    Shader _shader;
};

}
}

#endif

#ifndef ARK_RENDERER_GLES30_IMPL_GL_SNIPPET_BIND_VERTEX_ARRAY_H_
#define ARK_RENDERER_GLES30_IMPL_GL_SNIPPET_BIND_VERTEX_ARRAY_H_

#include "core/types/shared_ptr.h"

#include "renderer/inf/gl_snippet.h"

namespace ark {
namespace gles30 {

class BindVertexArray : public GLSnippet {
public:
    BindVertexArray(const sp<GLResource>& vertexArray);

    virtual void preDraw(GraphicsContext& graphicsContext, const GLShader& shader, const GLSnippetContext& context) override;

private:
    sp<GLResource> _vertex_array;
};

}
}

#endif

#ifndef ARK_RENDERER_OPENGL_ES30_GL_SNIPPET_BIND_VERTEX_ARRAY_H_
#define ARK_RENDERER_OPENGL_ES30_GL_SNIPPET_BIND_VERTEX_ARRAY_H_

#include "core/types/shared_ptr.h"

#include "renderer/inf/gl_snippet.h"
#include "renderer/base/gl_shader_preprocessor.h"

namespace ark {
namespace gles30 {

class BindVertexArray : public GLSnippet {
public:
    BindVertexArray(const sp<GLResource>& vertexArray);

    virtual void preDraw(GraphicsContext& graphicsContext, const GLPipeline& shader, const GLDrawingContext& context) override;
    virtual void postDraw(GraphicsContext& graphicsContext) override;

private:
    sp<GLResource> _vertex_array;

};

}
}

#endif

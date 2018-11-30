#ifndef ARK_RENDERER_OPENGL_ES20_GL_SNIPPET_BIND_ATTRIBUTES_H_
#define ARK_RENDERER_OPENGL_ES20_GL_SNIPPET_BIND_ATTRIBUTES_H_

#include "core/types/shared_ptr.h"

#include "renderer/inf/gl_snippet.h"

namespace ark {
namespace gles20 {

class BindAttributes : public GLSnippet {
public:

    virtual void preDraw(GraphicsContext& graphicsContext, const Shader& shader, const DrawingContext& context) override;

};

}
}

#endif

#ifndef ARK_RENDERER_IMPL_GL_SNIPPET_UPDATE_MODEL_MATRIX_H_
#define ARK_RENDERER_IMPL_GL_SNIPPET_UPDATE_MODEL_MATRIX_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "renderer/inf/gl_snippet.h"

namespace ark {

class ARK_API GLSnippetUpdateModelMatrix : public GLSnippet {
public:

    virtual void preDraw(GraphicsContext& graphicsContext, const GLShader& shader, const GLDrawingContext& context) override;

};

}

#endif

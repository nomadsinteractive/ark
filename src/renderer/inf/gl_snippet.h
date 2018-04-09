#ifndef ARK_RENDERER_INF_GL_SNIPPET_H_
#define ARK_RENDERER_INF_GL_SNIPPET_H_

#include "core/base/api.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API GLSnippet {
public:
    virtual ~GLSnippet() = default;

    virtual void preInitialize(GLShaderSource& source) {}

    virtual void preCompile(GraphicsContext& graphicsContext, GLShaderPreprocessorContext& context) {}
    virtual void preDraw(GraphicsContext& graphicsContext, const GLShader& shader, const GLDrawingContext& context) {}
    virtual void postDraw(GraphicsContext& graphicsContext) {}
};

}

#endif

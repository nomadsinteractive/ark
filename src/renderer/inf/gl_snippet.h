#ifndef ARK_RENDERER_INF_GL_SNIPPET_H_
#define ARK_RENDERER_INF_GL_SNIPPET_H_

#include "core/base/api.h"

#include "renderer/forwarding.h"
#include "renderer/util/gl_shader_preprocessor.h"

namespace ark {

class ARK_API GLSnippet {
public:
    virtual ~GLSnippet() = default;

    virtual void preCompile(GLShaderSource& source, GLShaderPreprocessor::Context& context) {}
    virtual void preDraw(GraphicsContext& graphicsContext, const GLShader& shader, const GLSnippetContext& context) {}
    virtual void postDraw(GraphicsContext& graphicsContext) {}
};

}

#endif

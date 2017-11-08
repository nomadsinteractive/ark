#ifndef ARK_RENDERER_GLES20_IMPL_GL_SNIPPET_POINT_SIZE_H_
#define ARK_RENDERER_GLES20_IMPL_GL_SNIPPET_POINT_SIZE_H_

#include "core/inf/builder.h"

#include "renderer/inf/gl_snippet.h"

namespace ark {

class GLSnippetPointSize : public GLSnippet {
public:
    virtual void preInitialize(GLShaderSource& source) override;
    virtual void preCompile(GraphicsContext& graphicsContext, GLShaderPreprocessor::Context& context) override;

//  [[plugin::builder::by-value("point-size")]]
    class DICTIONARY : public Builder<GLSnippet> {
    public:
        DICTIONARY() = default;

        virtual sp<GLSnippet> build(const sp<Scope>& args) override;

    };

};

}

#endif

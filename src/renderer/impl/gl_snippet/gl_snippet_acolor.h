#ifndef ARK_RENDERER_GLES20_IMPL_GL_SNIPPET_ACOLOR_H_
#define ARK_RENDERER_GLES20_IMPL_GL_SNIPPET_ACOLOR_H_

#include "core/inf/builder.h"

#include "renderer/inf/gl_snippet.h"

namespace ark {

class GLSnippetAcolor : public GLSnippet {
public:
    virtual void preInitialize(GLShaderSource& source) override;
    virtual void preCompile(GraphicsContext& graphicsContext, GLShaderPreprocessorContext& context) override;

//  [[plugin::builder::by-value("acolor")]]
    class DICTIONARY : public Builder<GLSnippet> {
    public:
        DICTIONARY() = default;

        virtual sp<GLSnippet> build(const sp<Scope>& args) override;

    };

};

}

#endif

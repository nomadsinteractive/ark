#ifndef ARK_RENDERER_GLES20_IMPL_GL_SNIPPET_ALPHA_H_
#define ARK_RENDERER_GLES20_IMPL_GL_SNIPPET_ALPHA_H_

#include "core/inf/builder.h"

#include "renderer/inf/gl_snippet.h"

namespace ark {

class GLSnippetAlpha : public GLSnippet {
public:
    virtual void preCompile(GLShaderSource& source, GLShaderPreprocessor::Context& context) override;

//  [[plugin::builder::by-value("alpha")]]
    class DICTIONARY : public Builder<GLSnippet> {
    public:
        DICTIONARY() = default;

        virtual sp<GLSnippet> build(const sp<Scope>& args) override;

    };

};

}

#endif

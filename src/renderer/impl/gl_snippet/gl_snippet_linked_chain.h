#ifndef ARK_RENDERER_IMPL_GL_SNIPPET_LINKED_CHAIN_H_
#define ARK_RENDERER_IMPL_GL_SNIPPET_LINKED_CHAIN_H_

#include "core/inf/builder.h"
#include "core/base/bean_factory.h"
#include "core/types/shared_ptr.h"

#include "renderer/inf/gl_snippet.h"

namespace ark {

class GLSnippetLinkedChain : public GLSnippet {
public:
    GLSnippetLinkedChain(const sp<GLSnippet>& delegate, const sp<GLSnippet>& next);

    virtual void preCompile(GLShaderSource& source, GLShaderPreprocessor::Context& context) override;
    virtual void preDraw(GraphicsContext& graphicsContext, const GLShader& shader, const GLSnippetContext& context) override;
    virtual void postDraw(GraphicsContext& graphicsContext) override;

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<GLSnippet> {
    public:
        DICTIONARY(BeanFactory& factory, const String& value);

        virtual sp<GLSnippet> build(const sp<Scope>& args) override;

    private:
        BeanFactory _factory;
        String _value;

    };

//  [[plugin::builder]]
    class BUILDER : public Builder<GLSnippet> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<GLSnippet> build(const sp<Scope>& args) override;

    private:
        sp<GLSnippet> _build(const document& manifest, const sp<Scope>& args);

    private:
        BeanFactory _factory;
        document _manifest;
    };

private:
    sp<GLSnippet> _delegate;
    sp<GLSnippet> _next;

};

}

#endif

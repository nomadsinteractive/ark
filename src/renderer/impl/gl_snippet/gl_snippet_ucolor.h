#ifndef ARK_RENDERER_IMPL_GL_SNIPPET_UCOLOR_H_
#define ARK_RENDERER_IMPL_GL_SNIPPET_UCOLOR_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/color.h"

#include "renderer/inf/gl_snippet.h"

namespace ark {

class GLSnippetUColor : public GLSnippet {
public:
    GLSnippetUColor(const sp<Vec4>& color);

    virtual void preInitialize(PipelineLayout& source) override;
    virtual void preCompile(GraphicsContext& graphicsContext, GLShaderPreprocessorContext& context) override;

//  [[plugin::builder::by-value("ucolor")]]
    class DICTIONARY : public Builder<GLSnippet> {
    public:
        DICTIONARY(BeanFactory& factory, const String& value);

        virtual sp<GLSnippet> build(const sp<Scope>&) override;

    private:
        sp<Builder<Vec4>> _color;

    };

private:
    sp<Vec4> _color;

};

}

#endif

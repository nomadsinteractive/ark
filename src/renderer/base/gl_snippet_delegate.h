#ifndef ARK_RENDERER_BASE_GL_SNIPPET_DELEGATE_H_
#define ARK_RENDERER_BASE_GL_SNIPPET_DELEGATE_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "renderer/inf/gl_snippet.h"

namespace ark {

class CoreGLSnippet;

class ARK_API GLSnippetDelegate : public GLSnippet {
public:
    GLSnippetDelegate(const sp<Shader>& shader);

    virtual void preInitialize(PipelineLayout& source) override;
    virtual void preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context) override;
    virtual void preDraw(GraphicsContext& graphicsContext, const Shader& shader, const DrawingContext& context) override;
    virtual void postDraw(GraphicsContext& graphicsContext) override;

    void link(const sp<GLSnippet>& snippet);
    template<typename T, typename... Args> sp<T> link(Args&&... args) {
        const sp<T> snippet = sp<T>::make(std::forward<Args>(args)...);
        link(snippet);
        return snippet;
    }

private:
    sp<GLSnippet> _core;
    sp<GLSnippet> _delegate;

    friend class CoreGLSnippet;
};

}

#endif

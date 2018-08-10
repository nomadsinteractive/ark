#ifndef ARK_RENDERER_IMPL_GL_SNIPPET_CLIP_PLANE_H_
#define ARK_RENDERER_IMPL_GL_SNIPPET_CLIP_PLANE_H_

#include "core/inf/builder.h"
#include "core/collection/list.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/inf/gl_snippet.h"

namespace ark {

class GLSnippetClipPlane : public GLSnippet {
public:
    virtual void preInitialize(GLShaderSource& source) override;
    virtual void preCompile(GraphicsContext& graphicsContext, GLShaderPreprocessorContext& context) override;
    virtual void preDraw(GraphicsContext& graphicsContext, const GLShader& shader, const GLDrawingContext& context) override;
    virtual void postDraw(GraphicsContext& graphicsContext) override;

//  [[plugin::builder("clip-plane")]]
    class BUILDER : public Builder<GLSnippet> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<GLSnippet> build(const sp<Scope>& args) override;

    private:
        List<std::pair<uint32_t, sp<Builder<Vec4>>>> _planes;
    };

private:
    bool _enabled;

    List<std::pair<uint32_t, sp<Vec4>>> _planes;

    friend class BUILDER;
};

}

#endif

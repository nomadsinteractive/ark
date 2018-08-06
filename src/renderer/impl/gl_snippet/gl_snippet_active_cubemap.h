#ifndef ARK_RENDERER_IMPL_GL_SNIPPET_ACTIVE_CUBEMAP_H_
#define ARK_RENDERER_IMPL_GL_SNIPPET_ACTIVE_CUBEMAP_H_

#include "core/types/shared_ptr.h"

#include "renderer/inf/gl_snippet.h"

namespace ark {

class GLSnippetActiveCubemap : public GLSnippet {
public:
    GLSnippetActiveCubemap(const sp<GLCubemap>& cubemap, uint32_t id = 0);

    virtual void preDraw(GraphicsContext& graphicsContext, const GLShader& shader, const GLDrawingContext& context) override;

private:
    sp<GLCubemap> _cubemap;
    uint32_t _id;
};

}

#endif

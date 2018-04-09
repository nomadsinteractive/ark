#ifndef ARK_RENDERER_GLES20_IMPL_GL_SNIPPET_ACTIVE_TEXTURE_H_
#define ARK_RENDERER_GLES20_IMPL_GL_SNIPPET_ACTIVE_TEXTURE_H_

#include "core/types/shared_ptr.h"

#include "renderer/inf/gl_snippet.h"

namespace ark {

class GLSnippetActiveTexture : public GLSnippet {
public:
    GLSnippetActiveTexture(const sp<GLTexture>& texture, uint32_t id = 0);

    virtual void preDraw(GraphicsContext& graphicsContext, const GLShader& shader, const GLDrawingContext& context) override;

private:
    sp<GLTexture> _texture;
    uint32_t _id;
};

}

#endif

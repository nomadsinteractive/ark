#ifndef ARK_RENDERER_GLES20_IMPL_GL_SNIPPET_TEXTURES_H_
#define ARK_RENDERER_GLES20_IMPL_GL_SNIPPET_TEXTURES_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/collection/list.h"
#include "core/types/shared_ptr.h"

#include "renderer/inf/gl_snippet.h"

namespace ark {

class ARK_API GLSnippetTextures : public GLSnippet {
public:

    void addTexture(uint32_t id, const sp<GLTexture>& texture);

    virtual void preDraw(GraphicsContext& graphicsContext, const GLShader& shader, const GLDrawingContext& context) override;

//  [[plugin::builder("textures")]]
    class BUILDER : public Builder<GLSnippet> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<GLSnippet> build(const sp<Scope>& args) override;

    private:
        List<std::pair<uint32_t, sp<Builder<GLTexture>>>> _textures;

    };

private:
    List<std::pair<uint32_t, sp<GLTexture>>> _textures;
};

}

#endif

#ifndef ARK_RENDERER_IMPL_GL_SNIPPET_ACTIVE_TEXTURE_H_
#define ARK_RENDERER_IMPL_GL_SNIPPET_ACTIVE_TEXTURE_H_

#include <vector>

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "renderer/inf/gl_snippet.h"

namespace ark {

class ARK_API GLSnippetActiveTexture : public GLSnippet {
public:
    GLSnippetActiveTexture();
    GLSnippetActiveTexture(const sp<GLTexture>& texture, uint32_t name = 0);

    virtual void preDraw(GraphicsContext& graphicsContext, const GLShader& shader, const GLDrawingContext& context) override;

//  [[plugin::builder("textures")]]
    class BUILDER : public Builder<GLSnippet> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<GLSnippet> build(const sp<Scope>& args) override;

    private:
        std::vector<std::pair<uint32_t, sp<Builder<GLTexture>>>> _textures;
    };

private:
    std::vector<std::pair<uint32_t, sp<GLTexture>>> _textures;

    friend class BUILDER;
};

}

#endif

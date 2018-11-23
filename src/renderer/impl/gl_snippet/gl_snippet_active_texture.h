#ifndef ARK_RENDERER_IMPL_GL_SNIPPET_ACTIVE_TEXTURE_H_
#define ARK_RENDERER_IMPL_GL_SNIPPET_ACTIVE_TEXTURE_H_

#include <vector>

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "renderer/inf/gl_snippet.h"

namespace ark {

class ARK_API GLSnippetActiveTexture : public GLSnippet {
private:
    struct Texture {
        Texture(const sp<GLTexture>& texture, uint32_t name);
        Texture(const sp<GLResource>& texture, uint32_t target, uint32_t name);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Texture);

        sp<GLResource> resource;
        uint32_t target;
        uint32_t name;
    };

public:
    GLSnippetActiveTexture();
    GLSnippetActiveTexture(const sp<GLTexture>& texture, uint32_t name = 0);
    GLSnippetActiveTexture(const sp<GLResource>& texture, uint32_t target, uint32_t name);

    virtual void preDraw(GraphicsContext& graphicsContext, const GLPipeline& shader, const GLDrawingContext& context) override;

//  [[plugin::builder("textures")]]
    class BUILDER : public Builder<GLSnippet> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<GLSnippet> build(const sp<Scope>& args) override;

    private:
        std::vector<std::pair<uint32_t, sp<Builder<GLTexture>>>> _textures;
    };

private:
    std::vector<Texture> _textures;

    friend class BUILDER;
};

}

#endif

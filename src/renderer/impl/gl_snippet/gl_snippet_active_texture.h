#ifndef ARK_RENDERER_IMPL_GL_SNIPPET_ACTIVE_TEXTURE_H_
#define ARK_RENDERER_IMPL_GL_SNIPPET_ACTIVE_TEXTURE_H_

#include <vector>

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "renderer/inf/gl_snippet.h"

namespace ark {

class ARK_API GLSnippetActiveTexture : public GLSnippet {
private:
    struct Slot {
        Slot(const sp<Texture>& texture, uint32_t name);
        Slot(const sp<Resource>& texture, uint32_t target, uint32_t name);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Slot);

        sp<Resource> resource;
        uint32_t target;
        uint32_t name;
    };

public:
    GLSnippetActiveTexture();
    GLSnippetActiveTexture(const sp<Texture>& texture, uint32_t name = 0);
    GLSnippetActiveTexture(const sp<Resource>& texture, uint32_t target, uint32_t name);

    virtual void preDraw(GraphicsContext& graphicsContext, const Shader& shader, const DrawingContext& context) override;

//  [[plugin::builder("textures")]]
    class BUILDER : public Builder<GLSnippet> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<GLSnippet> build(const sp<Scope>& args) override;

    private:
        std::vector<std::pair<uint32_t, sp<Builder<Texture>>>> _textures;
    };

private:
    std::vector<Slot> _textures;

    friend class BUILDER;
};

}

#endif

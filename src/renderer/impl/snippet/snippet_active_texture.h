#ifndef ARK_RENDERER_IMPL_SNIPPET_ACTIVE_TEXTURE_H_
#define ARK_RENDERER_IMPL_SNIPPET_ACTIVE_TEXTURE_H_

#include <vector>

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "renderer/base/texture.h"
#include "renderer/inf/snippet.h"

namespace ark {

class ARK_API SnippetActiveTexture : public Snippet {
private:
    struct Slot {
        Slot(const sp<Texture>& texture, uint32_t name);
        Slot(const sp<Resource>& texture, Texture::Type type, uint32_t name);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Slot);

        sp<Resource> resource;
        Texture::Type type;
        uint32_t name;
    };

public:
    SnippetActiveTexture();
    SnippetActiveTexture(const sp<Texture>& texture, uint32_t name = 0);
    SnippetActiveTexture(const sp<Resource>& texture, Texture::Type type, uint32_t name);

    virtual void preDraw(GraphicsContext& graphicsContext, const Shader& shader, const DrawingContext& context) override;

//  [[plugin::builder("textures")]]
    class BUILDER : public Builder<Snippet> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Snippet> build(const sp<Scope>& args) override;

    private:
        std::vector<std::pair<uint32_t, sp<Builder<Texture>>>> _textures;
    };

private:
    std::vector<Slot> _textures;

    friend class BUILDER;
};

}

#endif

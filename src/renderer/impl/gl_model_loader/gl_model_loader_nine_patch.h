#ifndef ARK_RENDERER_IMPL_GL_MODEL_LOADER_GL_MODEL_LOADER_NINE_PATCH_H_
#define ARK_RENDERER_IMPL_GL_MODEL_LOADER_GL_MODEL_LOADER_NINE_PATCH_H_

#include "core/inf/builder.h"
#include "core/collection/by_index.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/rect.h"

#include "renderer/forwarding.h"
#include "renderer/inf/gl_model_loader.h"

namespace ark {

class GLModelLoaderNinePatch : public GLModelLoader {
private:
    struct Item {
        Item(const Rect& bounds, const Rect& stretching, uint32_t textureWidth, uint32_t textureHeight);
        Item(const Item& other) = default;
        Item() = default;

        uint16_t _x[4];
        uint16_t _y[4];

        Rect paddings;
    };

public:
    GLModelLoaderNinePatch(const document& manifest, const sp<Atlas>& atlas);

    virtual uint32_t estimateVertexCount(uint32_t renderObjectCount) override;
    virtual void loadVertices(GLModelBuffer& buf, uint32_t type, const V& size) override;

    virtual GLBuffer getPredefinedIndexBuffer(GLResourceManager& glResourceManager, uint32_t renderObjectCount) override;

//  [[plugin::builder("nine-patch")]]
    class BUILDER : public Builder<GLModelLoader> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<GLModelLoader> build(const sp<Scope>& args) override;

    private:
        document _manifest;
        sp<Builder<Atlas>> _atlas;
    };

private:
    sp<Atlas> _atlas;
    ByIndex<Item> _nine_patch_items;
};

}

#endif
#ifndef ARK_RENDERER_IMPL_MODEL_LOADER_MODEL_LOADER_NINE_PATCH_H_
#define ARK_RENDERER_IMPL_MODEL_LOADER_MODEL_LOADER_NINE_PATCH_H_

#include "core/collection/by_index.h"
#include "core/types/shared_ptr.h"

#include "renderer/inf/model_loader.h"

namespace ark {

class ModelLoaderNinePatch : public ModelLoader {
private:
    struct Item {
        Item(const Rect& bounds, const Rect& patches, uint32_t textureWidth, uint32_t textureHeight);
        Item(const Item& other) = default;
        Item() = default;

        uint16_t _x[4];
        uint16_t _y[4];

        Rect _paddings;
    };

public:
    ModelLoaderNinePatch(const document& manifest, const sp<Atlas>& atlas);

    virtual Model load(int32_t type) override;

    class BUILDER : public Builder<ModelLoader> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<ModelLoader> build(const Scope& args) override;

    private:
        document _manifest;

        sp<Builder<Atlas>> _atlas;

    };

private:
    static sp<Model> makeUnitModel();
    static sp<Array<V3>> makeVertices(const Rect& bounds);

    void importAtlasItem(int32_t type, const Rect& paddings, uint32_t textureWidth, uint32_t textureHeight);

    Rect getPatches(const Rect& paddings, const Rect& bounds) const;

private:
    sp<Atlas> _atlas;

    ByIndex<Item> _nine_patch_items;
};

}

#endif

#ifndef ARK_RENDERER_IMPL_MODEL_LOADER_MODEL_LOADER_NINE_PATCH_H_
#define ARK_RENDERER_IMPL_MODEL_LOADER_MODEL_LOADER_NINE_PATCH_H_

#include "core/types/shared_ptr.h"

#include "renderer/inf/model_loader.h"

namespace ark {

class ModelLoaderNinePatch : public ModelLoader {
public:
    ModelLoaderNinePatch(const document& manifest, const sp<Atlas>& atlas);

    virtual void initialize(ShaderBindings& shaderBindings) override;
    virtual void postSnapshot(RenderController& renderController, RenderLayer::Snapshot& snapshot) override;

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
    void importAtlasItem(int32_t type, const Rect& paddings, uint32_t textureWidth, uint32_t textureHeight);

    Rect getPatches(const Rect& paddings, const Rect& bounds) const;

private:
    sp<Atlas> _atlas;

    std::unordered_map<int32_t, sp<Vertices>> _vertices;
};

}

#endif

#ifndef ARK_RENDERER_IMPL_MODEL_LOADER_MODEL_LOADER_NINE_PATCH_H_
#define ARK_RENDERER_IMPL_MODEL_LOADER_MODEL_LOADER_NINE_PATCH_H_

#include "core/types/shared_ptr.h"

#include "renderer/base/atlas.h"
#include "renderer/inf/model_loader.h"

namespace ark {

class ModelLoaderNinePatch : public ModelLoader {
public:
    ModelLoaderNinePatch(const document& manifest, const sp<Atlas>& atlas);

    virtual sp<RenderCommandComposer> makeRenderCommandComposer() override;

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

//  [[plugin::builder("nine-patch")]]
    class ATLAS_IMPORTER_BUILDER : public Builder<Atlas::Importer> {
    public:
        ATLAS_IMPORTER_BUILDER() = default;

        virtual sp<Atlas::Importer> build(const Scope& args) override;
    };

private:
//    void importAtlasItem(int32_t type, const Rect& paddings, uint32_t textureWidth, uint32_t textureHeight);

//    Rect getPatches(const Rect& paddings, const Rect& bounds) const;

    struct NinePatchVertices {

        void import(Atlas& atlas, const document& manifest);

        void add(int32_t type, uint32_t textureWidth, uint32_t textureHeight, const Rect& paddings, const Atlas& atlas);
        void add(int32_t type, uint32_t textureWidth, uint32_t textureHeight, const Rect& paddings, const Rect& bounds);

        Rect getPatches(const Rect& paddings, const Rect& bounds) const;

        std::unordered_map<int32_t, sp<Vertices>> _vertices;
    };

    class NinePatchAtlasImporter : public Atlas::Importer {
    public:
        virtual void import(Atlas& atlas, const document& manifest) override;
    };

private:
    sp<Atlas> _atlas;
    sp<NinePatchVertices> _vertices;

    friend class NinePatchAtlasImporter;

};

}

#endif

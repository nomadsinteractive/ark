#ifndef ARK_RENDERER_IMPL_MODEL_LOADER_MODEL_LOADER_NINE_PATCH_H_
#define ARK_RENDERER_IMPL_MODEL_LOADER_MODEL_LOADER_NINE_PATCH_H_

#include "core/types/shared_ptr.h"

#include "renderer/base/atlas.h"
#include "renderer/inf/model_loader.h"

namespace ark {

class ModelLoaderNinePatch : public ModelLoader {
public:
    ModelLoaderNinePatch(sp<Atlas> atlas);

    virtual sp<RenderCommandComposer> makeRenderCommandComposer() override;

    virtual void initialize(ShaderBindings& shaderBindings) override;
    virtual void postSnapshot(RenderController& renderController, RenderLayer::Snapshot& snapshot) override;

    virtual Model loadModel(int32_t type) override;

//  [[plugin::builder("nine-patch")]]
    class ATLAS_IMPORTER_BUILDER : public Builder<Atlas::Importer> {
    public:
        ATLAS_IMPORTER_BUILDER(const document& manifest);

        virtual sp<Atlas::Importer> build(const Scope& args) override;

    private:
        document _manifest;
    };

//  [[plugin::builder::by-value("nine-patch")]]
    class BUILDER : public Builder<ModelLoader> {
    public:
        BUILDER(BeanFactory& factory, const String& atlas);

        virtual sp<ModelLoader> build(const Scope& args) override;

    private:
        sp<Builder<Atlas>> _atlas;

    };

private:
    struct NinePatchVertices {

        void import(Atlas& atlas, const document& manifest);

        void add(int32_t type, uint32_t textureWidth, uint32_t textureHeight, const Rect& paddings, const Atlas& atlas);
        void add(int32_t type, uint32_t textureWidth, uint32_t textureHeight, const Rect& paddings, const Rect& bounds);

        sp<Vertices> makeNinePatchVertices(uint32_t textureWidth, uint32_t textureHeight, const Rect& paddings, const Rect& bounds) const;

        std::unordered_map<int32_t, sp<Vertices>> _vertices;
    };

    class NinePatchAtlasImporter : public Atlas::Importer {
    public:
        NinePatchAtlasImporter(document manifest);

        virtual void import(Atlas& atlas) override;

    private:
        document _manifest;
    };

private:
    sp<Atlas> _atlas;
    sp<NinePatchVertices> _vertices;

    friend class NinePatchAtlasImporter;

};

}

#endif

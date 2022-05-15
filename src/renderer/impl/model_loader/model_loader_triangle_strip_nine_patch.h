#ifndef ARK_RENDERER_IMPL_MODEL_LOADER_MODEL_LOADER_TRIANGLE_STRIP_NINE_PATCH_H_
#define ARK_RENDERER_IMPL_MODEL_LOADER_MODEL_LOADER_TRIANGLE_STRIP_NINE_PATCH_H_

#include "core/inf/storage.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/model.h"
#include "renderer/inf/model_loader.h"
#include "renderer/impl/importer/atlas_importer_nine_patch.h"

namespace ark {

class ModelLoaderTriangleStripNinePatch : public ModelLoader {
public:
    ModelLoaderTriangleStripNinePatch(sp<Atlas> atlas);

    virtual sp<RenderCommandComposer> makeRenderCommandComposer() override;

    virtual void initialize(ShaderBindings& shaderBindings) override;
    virtual void postSnapshot(RenderController& renderController, RenderLayer::Snapshot& snapshot) override;

    virtual sp<Model> loadModel(int32_t type) override;

//  [[plugin::builder::by-value("triangle-strip-nine-patch")]]
    class BUILDER : public Builder<ModelLoader> {
    public:
        BUILDER(BeanFactory& factory, const String& atlas);

        virtual sp<ModelLoader> build(const Scope& args) override;

    private:
        sp<Builder<Atlas>> _atlas;
    };

private:
    sp<Atlas> _atlas;
    sp<AtlasImporterNinePatch::Attachment> _nine_patch_attachment;
    Model _unit_model;
};

}

#endif

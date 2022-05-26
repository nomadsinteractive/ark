#ifndef ARK_RENDERER_IMPL_MODEL_LOADER_MODEL_LOADER_NINE_PATCH_TRIANGLE_STRIPS_H_
#define ARK_RENDERER_IMPL_MODEL_LOADER_MODEL_LOADER_NINE_PATCH_TRIANGLE_STRIPS_H_

#include "core/inf/storage.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/atlas.h"
#include "renderer/base/model.h"
#include "renderer/inf/model_loader.h"

namespace ark {

class ModelLoaderNinePatchTriangleStrips : public ModelLoader {
public:
    ModelLoaderNinePatchTriangleStrips(sp<Atlas> atlas);

    virtual sp<RenderCommandComposer> makeRenderCommandComposer() override;

    virtual void initialize(ShaderBindings& shaderBindings) override;

    virtual sp<Model> loadModel(int32_t type) override;

//  [[plugin::builder::by-value("nine-patch-triangle-strips")]]
    class BUILDER : public Builder<ModelLoader> {
    public:
        BUILDER(BeanFactory& factory, const String& atlas);

        virtual sp<ModelLoader> build(const Scope& args) override;

    private:
        sp<Builder<Atlas>> _atlas;
    };

private:
    sp<Atlas> _atlas;
    sp<Atlas::AttachmentNinePatch> _nine_patch_attachment;
    Model _unit_model;
};

}

#endif

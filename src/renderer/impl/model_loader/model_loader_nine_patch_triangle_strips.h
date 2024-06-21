#pragma once

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/atlas.h"
#include "renderer/base/model.h"
#include "renderer/inf/model_loader.h"

namespace ark {

class ModelLoaderNinePatchTriangleStrips : public ModelLoader {
public:
    ModelLoaderNinePatchTriangleStrips(sp<Atlas> atlas);

    sp<RenderCommandComposer> makeRenderCommandComposer() override;
    void initialize(ShaderBindings& shaderBindings) override;
    sp<Model> loadModel(int32_t type) override;

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
    sp<Model> _unit_model;
};

}

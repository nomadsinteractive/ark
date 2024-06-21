#pragma once

#include "core/inf/storage.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/atlas.h"
#include "renderer/inf/model_loader.h"

namespace ark {

class ModelLoaderNinePatchQuads final : public ModelLoader {
public:
    ModelLoaderNinePatchQuads(sp<Atlas> atlas);

    sp<RenderCommandComposer> makeRenderCommandComposer() override;
    void initialize(ShaderBindings& shaderBindings) override;
    sp<Model> loadModel(int32_t type) override;

//  [[plugin::builder::by-value("nine-patch-quads")]]
    class BUILDER : public Builder<ModelLoader> {
    public:
        BUILDER(BeanFactory& factory, const String& atlas);

        virtual sp<ModelLoader> build(const Scope& args) override;

    private:
        sp<Builder<Atlas>> _atlas;
    };

//  [[plugin::builder("nine-patch-quads")]]
    class BUILDER_IMPL : public Builder<ModelLoader> {
    public:
        BUILDER_IMPL(BeanFactory& factory, const document& manifest);

        sp<ModelLoader> build(const Scope& args) override;

    private:
        BUILDER _builder;
    };

private:
    sp<Atlas> _atlas;
    sp<Atlas::AttachmentNinePatch> _nine_patch_attachment;
    sp<Model> _unit_model;
};

}

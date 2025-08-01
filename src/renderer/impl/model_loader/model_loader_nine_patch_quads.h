#pragma once

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/atlas.h"
#include "renderer/inf/model_loader.h"

namespace ark {

class ModelLoaderNinePatchQuads final : public ModelLoader {
public:
    ModelLoaderNinePatchQuads(sp<Atlas> atlas);

    sp<DrawingContextComposer> makeRenderCommandComposer(const Shader& shader) override;
    sp<Model> loadModel(int32_t type) override;

//  [[plugin::builder::by-value("nine-patch-quads")]]
    class BUILDER final : public Builder<ModelLoader> {
    public:
        BUILDER(BeanFactory& factory, const String& atlas);

        sp<ModelLoader> build(const Scope& args) override;

    private:
        sp<Builder<Atlas>> _atlas;
    };

//  [[plugin::builder("nine-patch-quads")]]
    class BUILDER_IMPL final : public Builder<ModelLoader> {
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
    bool _is_lhs;
};

}

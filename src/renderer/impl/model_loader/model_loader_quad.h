#pragma once

#include "core/types/shared_ptr.h"

#include "renderer/inf/model_loader.h"

namespace ark {

class ModelLoaderQuad final : public ModelLoader {
public:
    ModelLoaderQuad(sp<Atlas> atlas);

    sp<RenderCommandComposer> makeRenderCommandComposer(const Shader& shader) override;
    sp<Model> loadModel(int32_t type) override;

//  [[plugin::builder::by-value("quad")]]
    class BUILDER final : public Builder<ModelLoader> {
    public:
        BUILDER(BeanFactory& factory, const String& atlas);

        sp<ModelLoader> build(const Scope& args) override;

    private:
        sp<Builder<Atlas>> _atlas;
    };

private:
    sp<Atlas> _atlas;
    sp<Model> _unit_model;
};

}

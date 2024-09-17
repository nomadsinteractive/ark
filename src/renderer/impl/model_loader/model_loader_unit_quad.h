#pragma once

#include "core/types/shared_ptr.h"

#include "renderer/inf/model_loader.h"

namespace ark {

class ModelLoaderUnitQuad final : public ModelLoader {
public:
    ModelLoaderUnitQuad(sp<Texture> texture);

    sp<RenderCommandComposer> makeRenderCommandComposer(const Shader& shader) override;
    sp<Model> loadModel(int32_t type) override;

//  [[plugin::builder::by-value("unit-quad")]]
    class BUILDER final : public Builder<ModelLoader> {
    public:
        BUILDER(BeanFactory& factory, const String& texture);

        sp<ModelLoader> build(const Scope& args) override;

    private:
        sp<Builder<Texture>> _texture;
    };

private:
    sp<Atlas> _atlas;
    sp<Model> _unit_model;
};

}

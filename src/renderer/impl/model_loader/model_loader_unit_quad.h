#pragma once

#include "core/types/shared_ptr.h"

#include "renderer/inf/model_loader.h"

namespace ark {

class ModelLoaderUnitQuad final : public ModelLoader {
public:
    ModelLoaderUnitQuad();

    sp<RenderCommandComposer> makeRenderCommandComposer(const Shader& shader) override;
    sp<Model> loadModel(int32_t type) override;

//  [[plugin::builder::by-value("unit-quad")]]
    class BUILDER final : public Builder<ModelLoader> {
    public:
        BUILDER() = default;

        sp<ModelLoader> build(const Scope& args) override;

    };

private:
    sp<Model> _unit_model;
};

}

#pragma once

#include "core/types/shared_ptr.h"

#include "renderer/base/model.h"
#include "renderer/inf/model_loader.h"

namespace ark {

class ModelLoaderQuad : public ModelLoader {
public:
    ModelLoaderQuad(sp<Atlas> atlas);

    sp<RenderCommandComposer> makeRenderCommandComposer() override;

    void initialize(ShaderBindings& shaderBindings) override;

    sp<Model> loadModel(int32_t type) override;

//  [[plugin::builder::by-value("quad")]]
    class MAKER : public Builder<ModelLoader> {
    public:
        MAKER(BeanFactory& factory, const String& atlas);

        virtual sp<ModelLoader> build(const Scope& args) override;

    private:
        sp<Builder<Atlas>> _atlas;

    };

private:
    sp<Atlas> _atlas;
    Model _unit_model;
};

}

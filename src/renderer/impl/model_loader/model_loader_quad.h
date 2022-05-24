#ifndef ARK_RENDERER_IMPL_MODEL_LOADER_MODEL_LOADER_QUAD_H_
#define ARK_RENDERER_IMPL_MODEL_LOADER_MODEL_LOADER_QUAD_H_

#include "core/types/shared_ptr.h"

#include "renderer/base/model.h"
#include "renderer/inf/model_loader.h"

namespace ark {

class ModelLoaderQuad : public ModelLoader {
public:
    ModelLoaderQuad(sp<Atlas> atlas);

    virtual sp<RenderCommandComposer> makeRenderCommandComposer() override;

    virtual void initialize(ShaderBindings& shaderBindings) override;

    virtual sp<Model> loadModel(int32_t type) override;

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

#endif

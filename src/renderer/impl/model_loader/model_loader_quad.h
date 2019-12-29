#ifndef ARK_RENDERER_IMPL_MODEL_LOADER_MODEL_LOADER_QUAD_H_
#define ARK_RENDERER_IMPL_MODEL_LOADER_MODEL_LOADER_QUAD_H_

#include "core/types/shared_ptr.h"

#include "renderer/base/model.h"
#include "renderer/inf/model_loader.h"

namespace ark {

class ModelLoaderQuad : public ModelLoader {
public:
    ModelLoaderQuad(const sp<Atlas>& atlas);

    virtual Model load(int32_t type) override;

    class BUILDER : public Builder<ModelLoader> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<ModelLoader> build(const Scope& args) override;

    private:
        sp<Builder<Atlas>> _atlas;

    };

private:
    static sp<Model> makeUnitModel();

private:
    sp<Atlas> _atlas;

    std::unordered_map<int32_t, Model> _models;
};

}

#endif

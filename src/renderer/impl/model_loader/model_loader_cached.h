#ifndef ARK_RENDERER_IMPL_MODEL_LOADER_MODEL_LOADER_CACHED_H_
#define ARK_RENDERER_IMPL_MODEL_LOADER_MODEL_LOADER_CACHED_H_

#include <unordered_map>

#include "core/types/shared_ptr.h"

#include "renderer/inf/model_loader.h"

namespace ark {

class ModelLoaderCached : public ModelLoader {
public:
    ModelLoaderCached(sp<ModelLoader> delegate);

    virtual sp<RenderCommandComposer> makeRenderCommandComposer() override;
    virtual void initialize(ShaderBindings& shaderBindings) override;
    virtual sp<Model> loadModel(int32_t type) override;

    static sp<ModelLoader> ensureCached(sp<ModelLoader> delegate);

private:
    sp<ModelLoader> _delegate;
    std::unordered_map<int32_t, sp<Model>> _cached_models;
};

}

#endif

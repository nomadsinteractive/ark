#pragma once

#include <unordered_map>

#include "core/types/shared_ptr.h"

#include "renderer/inf/model_loader.h"

namespace ark {

class ModelLoaderCached final : public ModelLoader {
public:
    ModelLoaderCached(sp<ModelLoader> delegate);

    sp<RenderCommandComposer> makeRenderCommandComposer(const Shader& shader) override;
    sp<Model> loadModel(int32_t type) override;

    static sp<ModelLoader> ensureCached(sp<ModelLoader> delegate);

private:
    sp<ModelLoader> _delegate;
    std::unordered_map<int32_t, sp<Model>> _cached_models;
};

}

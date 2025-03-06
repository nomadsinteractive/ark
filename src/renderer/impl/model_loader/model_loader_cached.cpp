#include "renderer/impl/model_loader/model_loader_cached.h"

#include "renderer/base/model.h"

namespace ark {

ModelLoaderCached::ModelLoaderCached(sp<ModelLoader> delegate)
    : ModelLoader(delegate->renderMode(), delegate->texture(), delegate->trait()), _delegate(std::move(delegate))
{
}

sp<DrawingContextComposer> ModelLoaderCached::makeRenderCommandComposer(const Shader& shader)
{
    return _delegate->makeRenderCommandComposer(shader);
}

sp<Model> ModelLoaderCached::loadModel(int32_t type)
{
    if(const auto iter = _cached_models.find(type); !(iter == _cached_models.end() || iter->second->isDiscarded()))
        return iter->second;

    sp<Model>& model = _cached_models[type];
    model = _delegate->loadModel(type);
    return model;
}

sp<ModelLoader> ModelLoaderCached::ensureCached(sp<ModelLoader> delegate)
{
    return !delegate || delegate->trait() & MODEL_TRAIT_DISALLOW_CACHE ? delegate : sp<ModelLoader>::make<ModelLoaderCached>(std::move(delegate));
}

}

#include "renderer/impl/model_loader/model_loader_cached.h"

#include "renderer/base/model.h"

namespace ark {

ModelLoaderCached::ModelLoaderCached(sp<ModelLoader> delegate)
    : ModelLoader(delegate->renderMode()), _delegate(std::move(delegate))
{
}

sp<RenderCommandComposer> ModelLoaderCached::makeRenderCommandComposer()
{
    return _delegate->makeRenderCommandComposer();
}

void ModelLoaderCached::initialize(PipelineBindings& pipelineBindings)
{
    _delegate->initialize(pipelineBindings);
}

sp<Model> ModelLoaderCached::loadModel(int32_t type)
{
    const auto iter = _cached_models.find(type);
    if(!(iter == _cached_models.end() || iter->second->isDiscarded()))
        return iter->second;

    sp<Model>& model = _cached_models[type];
    model = _delegate->loadModel(type);
    return model;
}

sp<ModelLoader> ModelLoaderCached::ensureCached(sp<ModelLoader> delegate)
{
    return delegate ? sp<ModelLoader>::make<ModelLoaderCached>(std::move(delegate)) : delegate;
}

}

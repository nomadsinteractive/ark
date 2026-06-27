#include "renderer/inf/model_loader.h"

#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/texture.h"
#include "renderer/impl/model_loader/model_loader_quad.h"

namespace ark {

ModelLoader::ModelLoader(const enums::DrawMode renderMode, sp<Texture> texture, const ModelTrait trait)
    : _render_mode(renderMode), _texture(std::move(texture)), _trait(trait) {
}

enums::DrawMode ModelLoader::renderMode() const
{
    return _render_mode;
}

void ModelLoader::bind(const PipelineBindings& pipelineBindings) const
{
    if(_texture)
        pipelineBindings.bindSampler(_texture);
}

const sp<Texture>& ModelLoader::texture() const
{
    return _texture;
}

ModelLoader::ModelTrait ModelLoader::trait() const
{
    return _trait;
}

sp<ModelLoader> ModelLoader::quad(sp<Atlas> atlas)
{
    return sp<ModelLoader>::make<ModelLoaderQuad>(atlas);
}

}

#include "renderer/inf/model_loader.h"

#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/pipeline_descriptor.h"

namespace ark {

ModelLoader::ModelLoader(Enum::RenderMode renderMode, sp<Texture> texture)
    : _render_mode(renderMode), _texture(std::move(texture)) {
}

Enum::RenderMode ModelLoader::renderMode() const
{
    return _render_mode;
}

void ModelLoader::bind(const PipelineBindings& pipelineBindings) const
{
    if(_texture)
        pipelineBindings.pipelineDescriptor()->bindSampler(_texture);
}

const sp<Texture>& ModelLoader::texture() const
{
    return _texture;
}

}

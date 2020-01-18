#include "renderer/impl/model_loader/model_loader_quad.h"

#include "renderer/base/atlas.h"
#include "renderer/base/model.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/impl/vertices/vertices_quad.h"
#include "renderer/util/element_util.h"

namespace ark {

ModelLoaderQuad::ModelLoaderQuad(const sp<Atlas>& atlas)
    : ModelLoader(RenderModel::RENDER_MODE_TRIANGLES), _atlas(atlas)
{
}

void ModelLoaderQuad::initialize(ShaderBindings& shaderBindings)
{
    shaderBindings.pipelineBindings()->bindSampler(_atlas->texture());
}

void ModelLoaderQuad::postSnapshot(RenderController& /*renderController*/, RenderLayer::Snapshot& /*snapshot*/)
{
}

Model ModelLoaderQuad::load(int32_t type)
{
    const Atlas::Item& texCoord = _atlas->at(type);
    const V2& size = texCoord.size();
    return Model(nullptr, sp<VerticesQuad>::make(texCoord), {V3(size, 0), V3(size, 0), V3(0)});
}

ModelLoaderQuad::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _atlas(factory.ensureConcreteClassBuilder<Atlas>(manifest, Constants::Attributes::ATLAS))
{
}

sp<ModelLoader> ModelLoaderQuad::BUILDER::build(const Scope& args)
{
    return sp<ModelLoaderQuad>::make(_atlas->build(args));
}

}

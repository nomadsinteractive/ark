#include "renderer/impl/model_loader/model_loader_point.h"

#include "renderer/base/atlas.h"
#include "renderer/base/model.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/impl/vertices/vertices_point.h"

namespace ark {

ModelLoaderPoint::ModelLoaderPoint(const sp<Atlas>& atlas)
    : ModelLoader(RenderModel::RENDER_MODE_POINTS, makeUnitModel()), _atlas(atlas)
{
}

void ModelLoaderPoint::initialize(ShaderBindings& shaderBindings)
{
    shaderBindings.pipelineBindings()->bindSampler(_atlas->texture());
}

void ModelLoaderPoint::postSnapshot(RenderController& /*renderController*/, RenderLayer::Snapshot& /*snapshot*/)
{
}

Model ModelLoaderPoint::load(int32_t type)
{
    const Atlas::Item& texCoord = _atlas->at(type);
    const V2& size = texCoord.size();
    return Model(nullptr, sp<VerticesPoint>::make(texCoord), {V3(size, 0), V3(size, 0), V3(0)});
}

sp<Model> ModelLoaderPoint::makeUnitModel()
{
    return sp<Model>::make(sp<IndexArray::Fixed<1>>::make(std::initializer_list<element_index_t>({0})), sp<VerticesPoint>::make(), Metrics{V3(1.0f), V3(1.0f), V3()});
}

ModelLoaderPoint::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _atlas(factory.ensureConcreteClassBuilder<Atlas>(manifest, Constants::Attributes::ATLAS))
{
}

sp<ModelLoader> ModelLoaderPoint::BUILDER::build(const Scope& args)
{
    return sp<ModelLoaderPoint>::make(_atlas->build(args));
}

}

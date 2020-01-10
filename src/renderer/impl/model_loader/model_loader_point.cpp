#include "renderer/impl/model_loader/model_loader_point.h"

#include "renderer/base/atlas.h"
#include "renderer/base/model.h"
#include "renderer/impl/vertices/vertices_point.h"

namespace ark {

ModelLoaderPoint::ModelLoaderPoint(const sp<Atlas>& atlas)
    : ModelLoader(RenderModel::RENDER_MODE_POINTS, makeUnitModel()), _atlas(atlas)
{
}

void ModelLoaderPoint::initialize(ShaderBindings& shaderBindings)
{
}

void ModelLoaderPoint::postSnapshot(RenderController& /*renderController*/, RenderLayer::Snapshot& /*snapshot*/)
{
}

Model ModelLoaderPoint::load(int32_t type)
{
    const auto iter = _models.find(type);
    if(iter != _models.end())
        return iter->second;

    const Atlas::Item& texCoord = _atlas->at(type);
    const V2& size = texCoord.size();
    Model model(nullptr, sp<VerticesPoint>::make(texCoord), V3(size, 0));
    _models.insert(std::make_pair(type, model));
    return model;
}

sp<Model> ModelLoaderPoint::makeUnitModel()
{
    return sp<Model>::make(nullptr, sp<VerticesPoint>::make(), V3(1.0f));
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

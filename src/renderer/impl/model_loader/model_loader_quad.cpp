#include "renderer/impl/model_loader/model_loader_quad.h"

#include "renderer/base/atlas.h"
#include "renderer/base/model.h"
#include "renderer/impl/vertices/vertices_quad.h"

namespace ark {

ModelLoaderQuad::ModelLoaderQuad(const sp<Atlas>& atlas)
    : ModelLoader(RenderModel::RENDER_MODE_TRIANGLES, makeUnitModel()), _atlas(atlas)
{
}

Model ModelLoaderQuad::load(int32_t type)
{
    const auto iter = _models.find(type);
    if(iter != _models.end())
        return iter->second;

    const Atlas::Item& texCoord = _atlas->at(type);
    const V2& size = texCoord.size();
    Model model(nullptr, sp<VerticesQuad>::make(texCoord), V3(size, 0));
    _models.insert(std::make_pair(type, model));
    return model;
}

sp<Model> ModelLoaderQuad::makeUnitModel()
{
    return sp<Model>::make(nullptr, sp<VerticesQuad>::make(), V3(1.0f));
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

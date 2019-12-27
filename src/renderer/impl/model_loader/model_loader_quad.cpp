#include "renderer/impl/model_loader/model_loader_quad.h"

#include "renderer/base/atlas.h"
#include "renderer/base/model.h"

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
    sp<Array<Model::UV>> uvs = sp<Array<Model::UV>::Fixed<4>>::make(std::initializer_list<Model::UV>({{texCoord.ux(), texCoord.uy()}, {texCoord.ux(), texCoord.vy()},
                                                                                                      {texCoord.vx(), texCoord.uy()}, {texCoord.vx(), texCoord.vy()}}));
    Model model(nullptr, makeVertices(texCoord.bounds()), uvs, nullptr, nullptr, V3(size, 0));
    _models.insert(std::make_pair(type, model));
    return model;
}

sp<Model> ModelLoaderQuad::makeUnitModel()
{
    return sp<Model>::make(nullptr, makeVertices(Rect(0, 0, 1.0f, 1.0f)), nullptr, nullptr, nullptr, V3(1.0f));
}

sp<Array<V3>> ModelLoaderQuad::makeVertices(const Rect& bounds)
{
    return sp<Array<V3>::Fixed<4>>::make(std::initializer_list<V3>({{bounds.left(), bounds.top(), 0}, {bounds.left(), bounds.bottom(), 0},
                                                                    {bounds.right(), bounds.top(), 0}, {bounds.right(), bounds.bottom(), 0}}));
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

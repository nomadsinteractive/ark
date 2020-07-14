#include "renderer/base/model_bundle.h"

#include "core/base/bean_factory.h"
#include "core/util/conversions.h"

#include "renderer/base/atlas.h"
#include "renderer/base/texture_packer.h"
#include "renderer/inf/vertices.h"

namespace ark {

ModelBundle::ModelBundle(sp<Atlas> atlas)
    : _atlas(std::move(atlas)), _vertex_length(0), _index_length(0)
{
}

void ModelBundle::import(const sp<ResourceLoaderContext>& resourceLoaderContext, const document& manifest, ModelBundle::Importer& importer)
{
    bool hasModelMaps = false;
    TexturePacker texturePacker(resourceLoaderContext, _atlas->width(), _atlas->height(), false);
    for(const document& i : manifest->children())
    {
        int32_t type = Documents::ensureAttribute<int32_t>(i, Constants::Attributes::TYPE);
        const String& src = Documents::ensureAttribute(i, Constants::Attributes::SRC);
        for(const document& j : i->children("map"))
        {
            const String& mappingSrc = Documents::ensureAttribute(j, Constants::Attributes::SRC);
            const RectI rect = texturePacker.addBitmap(mappingSrc);
            _atlas->add(type, rect.left(), rect.top(), rect.right(), rect.bottom(), Rect(0, 0, 1.0f, 1.0f), V2(rect.width(), rect.height()), V2(0.5f, 0.5f));
            hasModelMaps = true;
        }

        const Rect bounds = _atlas && _atlas->has(type) ? _atlas->getItemUV(type) : Rect(0, 1.0f, 1.0f, 0);
        addModel(type, importer.import(src, bounds));
    }
    if(hasModelMaps)
        texturePacker.updateTexture(_atlas->texture());
}

ModelBundle::ModelInfo& ModelBundle::addModel(int32_t type, const Model& model)
{
    ModelInfo& modelInfo = _models[type];
    modelInfo = {model, _vertex_length, _index_length};
    _vertex_length += model.vertexLength();
    _index_length += model.indexLength();
    return modelInfo;
}

const ModelBundle::ModelInfo& ModelBundle::ensure(int32_t type) const
{
    const auto iter = _models.find(type);
    DCHECK(iter != _models.end(), "Model not found, type: %d", type);
    return iter->second;
}

Model ModelBundle::load(int32_t type) const
{
    return ensure(type)._model;
}

const Table<int32_t, ModelBundle::ModelInfo>& ModelBundle::models() const
{
    return _models;
}

size_t ModelBundle::vertexLength() const
{
    return _vertex_length;
}

size_t ModelBundle::indexLength() const
{
    return _index_length;
}

template<> ARK_API ModelBundle::MappingType Conversions::to<String, ModelBundle::MappingType>(const String& str)
{
    DCHECK(str == "albedo", "Only albedo mapping is supported");
    return ModelBundle::MAPPING_TYPE_ALBEDO;
}

}

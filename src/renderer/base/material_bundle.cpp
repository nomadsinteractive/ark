#include "renderer/base/material_bundle.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/util/documents.h"

#include "graphics/base/rect.h"
#include "graphics/util/max_rects_bin_pack.h"

#include "renderer/base/texture_packer.h"

namespace ark {

MaterialBundle::MaterialBundle(const sp<ResourceLoaderContext>& resourceLoaderContext, std::map<String, sp<Material>> materials, std::array<sp<Texture>, Material::TEXTURE_TYPE_LENGTH> textures)
    : _materials(std::move(materials))
{
    for(size_t i = 0; i < Material::TEXTURE_TYPE_LENGTH; ++i)
        if(textures[i])
            _texture_packers[i] = sp<TexturePacker>::make(resourceLoaderContext, std::move(textures[i]));

    MaxRectsBinPack binPack;

    for(const auto& i : _materials)
    {
        const sp<Material>& material = i.second;
        RectI packedRect;
        bool packed = false;
        for(size_t j = 0; j < Material::TEXTURE_TYPE_LENGTH; ++j)
            if(_texture_packers[j])
            {
                const sp<VariableWrapper<bitmap>>& bitmapProvider = material->getTexture(static_cast<Material::TextureType>(j));
                bitmap bitmap = bitmapProvider->val();
                if(bitmap)
                {
                    if(!packed)
                        packedRect = _texture_packers[j]->addBitmap(binPack, bitmap, bitmapProvider);
                    else
                        _texture_packers[j]->addPackedBitmap(packedRect.left(), packedRect.top(), bitmap, bitmapProvider);
                }
            }
    }

    for(const sp<TexturePacker>& i : _texture_packers)
        if(i)
            i->updateTexture();
}

sp<Material> MaterialBundle::getMaterial(const String& name) const
{
    const auto iter = _materials.find(name);
    return iter != _materials.end() ? iter->second : nullptr;
}

MaterialBundle::BUILDER::BUILDER(BeanFactory& beanFactory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _names(Documents::ensureAttributeList<String>(manifest, Constants::Attributes::NAME, "material")), _materials(beanFactory.getBuilderList<Material>(manifest, "material")),
      _types(Documents::ensureAttributeList<Material::TextureType>(manifest, Constants::Attributes::TYPE, Constants::Attributes::TEXTURE)), _textures(beanFactory.getBuilderList<Texture>(manifest, Constants::Attributes::TEXTURE)) {
}

sp<MaterialBundle> MaterialBundle::BUILDER::build(const Scope& args)
{
    std::map<String, sp<Material>> materials;
    DASSERT(_names.size() == _materials.size());
    for(size_t i = 0; i < _names.size(); ++i)
        materials.insert_or_assign(_names.at(i), _materials.at(i)->build(args));

    DASSERT(_types.size() == _textures.size());
    std::array<sp<Texture>, Material::TEXTURE_TYPE_LENGTH> textures;
    for(size_t i = 0; i < _types.size(); ++i)
        textures[_types.at(i)] = _textures.at(i)->build(args);
    return sp<MaterialBundle>::make(_resource_loader_context, std::move(materials), std::move(textures));
}

}

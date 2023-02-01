#include "renderer/base/material_bundle.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/util/documents.h"

#include "graphics/base/material.h"
#include "graphics/util/max_rects_bin_pack.h"

#include "renderer/base/texture_packer.h"

namespace ark {

static std::map<String, sp<Material>> _to_material_map(const std::vector<sp<Material>>& materials)
{
    std::map<String, sp<Material>> materialMap;
    for(const sp<Material>& i : materials)
        materialMap[i->name()] = i;
    return materialMap;
}


MaterialBundle::MaterialBundle(const std::vector<sp<Material>>& materials)
    : MaterialBundle(_to_material_map(materials), std::array<sp<Texture>, MaterialTexture::TYPE_LENGTH>())
{
}

MaterialBundle::MaterialBundle(std::map<String, sp<Material>> materials, std::array<sp<Texture>, MaterialTexture::TYPE_LENGTH> textures)
    : _materials(std::move(materials)), _width(0), _height(0)
{
    for(size_t i = 0; i < MaterialTexture::TYPE_LENGTH; ++i)
        if(textures[i])
        {
            if(_width == 0)
            {
                _width = textures[i]->width();
                _height = textures[i]->height();
            }
            _texture_packers[i] = sp<TexturePacker>::make(Ark::instance().applicationContext(), std::move(textures[i]));
        }

    MaxRectsBinPack binPack(_width, _height, false);
    std::vector<std::map<bitmap, RectI>> bitmapBounds(MaterialTexture::TYPE_LENGTH);

    for(const auto& i : _materials)
    {
        const sp<Material>& material = i.second;
        for(size_t j = 0; j < MaterialTexture::TYPE_LENGTH; ++j)
            if(_texture_packers[j])
            {
                const sp<MaterialTexture>& texture = material->getTexture(static_cast<MaterialTexture::Type>(j));
                bitmap bitmap = texture->bitmap();
                if(bitmap)
                {
                    const auto biter = bitmapBounds[j].find(bitmap);
                    if(biter == bitmapBounds[j].end())
                    {
                        const auto iter = _material_bounds.find(i.first);
                        if(iter == _material_bounds.end())
                            bitmapBounds[j][bitmap] = _material_bounds[i.first] = _texture_packers[j]->addBitmap(binPack, bitmap, texture->bitmapWrapper());
                        else
                        {
                            const RectI& packedBounds = iter->second;
                            _texture_packers[j]->addPackedBitmap(packedBounds.left(), packedBounds.top(), bitmap, texture->bitmapWrapper());
                        }
                    }
                    else
                        _material_bounds[i.first] = biter->second;
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

void MaterialBundle::addMaterial(String name, sp<Material> material)
{
    _materials[std::move(name)] = std::move(material);
}

Rect MaterialBundle::getMaterialUV(const String& name) const
{
    const auto iter = _material_bounds.find(name);
    if(iter == _material_bounds.end())
        return Rect(0, 1.0f, 1.0f, 0);

    const RectI& bounds = iter->second;
    float fwidth = static_cast<float>(_width);
    float fheight = static_cast<float>(_height);
    return Rect(bounds.left() / fwidth, bounds.bottom() / fheight, bounds.right() / fwidth, bounds.top() / fheight);
}

MaterialBundle::BUILDER::BUILDER(BeanFactory& beanFactory, const document& manifest)
    : _names(Documents::ensureAttributeList<String>(manifest, Constants::Attributes::NAME, "material")), _materials(beanFactory.makeBuilderList<Material>(manifest, "material")),
      _types(Documents::ensureAttributeList<MaterialTexture::Type>(manifest, Constants::Attributes::TYPE, Constants::Attributes::TEXTURE)), _textures(beanFactory.makeBuilderList<Texture>(manifest, Constants::Attributes::TEXTURE)) {
}

sp<MaterialBundle> MaterialBundle::BUILDER::build(const Scope& args)
{
    std::map<String, sp<Material>> materials;
    DASSERT(_names.size() == _materials.size());
    for(size_t i = 0; i < _names.size(); ++i)
        materials[_names.at(i)] = _materials.at(i)->build(args);

    DASSERT(_types.size() == _textures.size());
    std::array<sp<Texture>, MaterialTexture::TYPE_LENGTH> textures;
    for(size_t i = 0; i < _types.size(); ++i)
        textures[_types.at(i)] = _textures.at(i)->build(args);
    return sp<MaterialBundle>::make(std::move(materials), std::move(textures));
}

}

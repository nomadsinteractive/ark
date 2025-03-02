#include "renderer/base/material_bundle.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/util/documents.h"

#include "graphics/base/material.h"
#include "graphics/util/max_rects_bin_pack.h"

#include "renderer/base/texture_packer.h"

namespace ark {

namespace {

Table<String, sp<Material>> _to_material_map(const Vector<sp<Material>>& materials)
{
    Table<String, sp<Material>> materialMap;
    for(const sp<Material>& i : materials)
        materialMap[i->name()] = i;
    return materialMap;
}

}

MaterialBundle::MaterialBundle(const Vector<sp<Material>>& materials)
    : MaterialBundle(_to_material_map(materials), std::array<sp<Texture>, MaterialTexture::TYPE_LENGTH>())
{
}

MaterialBundle::MaterialBundle(Table<String, sp<Material>> materials, std::array<sp<Texture>, MaterialTexture::TYPE_LENGTH> textures)
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
    Vector<Map<bitmap, RectI>> bitmapBounds(MaterialTexture::TYPE_LENGTH);

    for(const auto& i : _materials)
    {
        const sp<Material>& material = i.second;
        for(size_t j = 0; j < MaterialTexture::TYPE_LENGTH; ++j)
            if(_texture_packers[j])
            {
                const sp<MaterialTexture>& texture = material->getTexture(static_cast<MaterialTexture::Type>(j));
                if(bitmap bitmap = texture->bitmap())
                {
                    if(const auto biter = bitmapBounds[j].find(bitmap); biter == bitmapBounds[j].end())
                    {
                        if(const auto iter = _material_bounds.find(i.first); iter == _material_bounds.end())
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

const Vector<sp<Material>>& MaterialBundle::materials() const
{
    return _materials.values();
}

sp<Material> MaterialBundle::getMaterial(const String& name) const
{
    const auto iter = _materials.find(name);
    return iter != _materials.end() ? iter->second : nullptr;
}

void MaterialBundle::addMaterial(String name, sp<Material> material)
{
    if(const auto iter = _materials.find(name); iter == _materials.end())
    {
        material->setId(_materials.size());
        _materials.push_back(std::move(name), std::move(material));
    }
    else
    {
        material->setId(iter->second->id());
        iter->second = std::move(material);
    }
}

sp<Material> MaterialBundle::addMaterial(String name)
{
    sp<Material> material = sp<Material>::make(_materials.size(), name);
    _materials.push_back(std::move(name), material);
    return material;
}

Rect MaterialBundle::getMaterialUV(const String& name) const
{
    const auto iter = _material_bounds.find(name);
    if(iter == _material_bounds.end())
        return {0, 1.0f, 1.0f, 0};

    const RectI& bounds = iter->second;
    const float fwidth = static_cast<float>(_width);
    const float fheight = static_cast<float>(_height);
    return {bounds.left() / fwidth, bounds.bottom() / fheight, bounds.right() / fwidth, bounds.top() / fheight};
}

MaterialBundle::BUILDER::BUILDER(BeanFactory& beanFactory, const document& manifest)
    : _names(Documents::ensureAttributeList<String>(manifest, constants::NAME, "material")), _materials(beanFactory.makeBuilderList<Material>(manifest, "material")),
      _types(Documents::ensureAttributeList<MaterialTexture::Type>(manifest, constants::TYPE, constants::TEXTURE)), _textures(beanFactory.makeBuilderList<Texture>(manifest, constants::TEXTURE)) {
}

sp<MaterialBundle> MaterialBundle::BUILDER::build(const Scope& args)
{
    Table<String, sp<Material>> materials;
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

#include "renderer/base/material_bundle.h"

#include "core/ark.h"
#include "core/impl/variable/variable_wrapper.h"

#include "graphics/base/material.h"
#include "graphics/util/max_rects_bin_pack.h"

#include "renderer/base/texture_packer.h"

namespace ark {

namespace {

Table<String, sp<Material>> toMaterialMap(const Vector<sp<Material>>& materials)
{
    Table<String, sp<Material>> materialMap;
    for(const sp<Material>& i : materials)
        materialMap[i->name()] = i;
    return materialMap;
}

Vector<sp<Bitmap>> getMaterialTextureImages(const Vector<sp<Material>>& materials, const MaterialTexture::Type type)
{
    Table<String, sp<Bitmap>> images;
    for(const sp<Material>& i : materials)
    {
        const sp<MaterialTexture>& texture = i->getTexture(type);
        if(images.find(texture->name()) != images.end())
            continue;

        if(sp<Bitmap> bitmap = texture->bitmap())
            images[texture->name()] = std::move(bitmap);
    }
    return std::move(images.values());
}

}

MaterialBundle::MaterialBundle(const Vector<sp<Material>>& materials)
    : _materials(toMaterialMap(materials))
{
}

const Vector<sp<Material>>& MaterialBundle::materials() const
{
    return _materials.values();
}

const Map<String, sp<Bitmap>>& MaterialBundle::images() const
{
    return _images;
}

Map<String, sp<Bitmap>>& MaterialBundle::images()
{
    return _images;
}

const std::array<sp<Texture>, MaterialTexture::TYPE_LENGTH>& MaterialBundle::textures() const
{
    return _textures;
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

void MaterialBundle::update()
{
     Vector<Map<bitmap, RectI>> bitmapBounds(MaterialTexture::TYPE_LENGTH);

     for(const auto& i : _materials)
     {
         const sp<Material>& material = i.second;
     }

     for(const sp<TexturePacker>& i : _texture_packers)
         if(i)
             i->updateTexture();
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

}

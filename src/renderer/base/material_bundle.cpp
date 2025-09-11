#include "renderer/base/material_bundle.h"

#include "core/ark.h"
#include "core/util/math.h"

#include "graphics/base/bitmap.h"
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

Table<String, sp<Bitmap>> getMaterialTextureImages(const Vector<sp<Material>>& materials, const MaterialTexture::Type type)
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
    return images;
}

sp<TexturePacker> makeTexturePackerForImages(const Vector<sp<Bitmap>>& images)
{
    uint32_t imageSize = 0;
    for(const sp<Bitmap>& i : images)
    {
        imageSize = std::max(imageSize, i->width());
        imageSize = std::max(imageSize, i->height());
    }
    imageSize = 1 << (Math::log2(imageSize) + 1);
    return sp<TexturePacker>::make(imageSize, imageSize);
}

}

MaterialBundle::MaterialBundle(const Vector<sp<Material>>& materials)
    : _materials(toMaterialMap(materials))
{
    update();
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

std::array<sp<Texture>, MaterialTexture::TYPE_LENGTH> MaterialBundle::textures() const
{
    std::array<sp<Texture>, MaterialTexture::TYPE_LENGTH> textures;
    for(size_t i = 0; i < MaterialTexture::TYPE_LENGTH; ++i)
        textures[i] = _texture_infos[i]._texture;
    return textures;
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
    sp<TexturePacker> texturePackers[MaterialTexture::TYPE_LENGTH];

    for(size_t i = 0; i < MaterialTexture::TYPE_LENGTH; ++i)
        if(Table<String, sp<Bitmap>> images = getMaterialTextureImages(_materials.values(), static_cast<MaterialTexture::Type>(i)); !images.empty())
        {
            texturePackers[i] = makeTexturePackerForImages(images.values());
            for(auto& [k, v] : images)
                texturePackers[i]->addBitmap(std::move(v), std::move(k));

            _texture_infos[i]._bounds.clear();
            for(const TexturePacker::PackedBitmap& j : texturePackers[i]->packedBitmaps())
                _texture_infos[i]._bounds[j._name] = j._uv;
        }

    for(size_t i = 0; i < MaterialTexture::TYPE_LENGTH; ++i)
        if(texturePackers[i])
        {
            if(_texture_infos[i]._texture)
                texturePackers[i]->updateTexture(_texture_infos[i]._texture);
            else
                _texture_infos[i]._texture = texturePackers[i]->createTexture();
        }
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

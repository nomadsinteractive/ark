#include "renderer/base/material_bundle.h"

#include <ranges>

#include "core/ark.h"
#include "core/util/math.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/material.h"
#include "graphics/components/size.h"
#include "graphics/util/max_rects_bin_pack.h"

#include "renderer/base/texture_packer.h"

namespace ark {

struct MaterialBundle::Stub {
    int32_t _texture_width = 0;
    int32_t _texture_height = 0;
};


class MaterialBundle::VariableMaterialUV final : public Variable<Rect> {
public:
    VariableMaterialUV(const sp<Stub>& stub, const RectI uv)
        : _stub(stub), _uv(uv) {
    }

    bool update(uint64_t timestamp) override
    {
        return false;
    }

    RectT<float> val() override
    {
        const float fwidth = static_cast<float>(_stub->_texture_width);
        const float fheight = static_cast<float>(_stub->_texture_height);
        return {_uv.left() / fwidth, _uv.bottom() / fheight, _uv.right() / fwidth, _uv.top() / fheight};
    }

private:
    sp<Stub> _stub;
    RectI _uv;
};

namespace {

Table<String, sp<Material>> toMaterialMap(const Vector<sp<Material>>& materials)
{
    Table<String, sp<Material>> materialMap;
    for(const sp<Material>& i : materials)
        materialMap[i->name()] = i;
    return materialMap;
}

Vector<std::pair<sp<Material>, sp<Bitmap>>> getMaterialImages(const Vector<sp<Material>>& materials, const MaterialTexture::Type type)
{
    Vector<std::pair<sp<Material>, sp<Bitmap>>> images;
    for(const sp<Material>& i : materials)
    {
        const sp<MaterialTexture>& texture = i->getTexture(type);
        if(sp<Bitmap> bitmap = texture->bitmap())
            images.emplace_back(i, std::move(bitmap));
    }
    return images;
}

sp<TexturePacker> makeTexturePackerForImages(const Vector<std::pair<sp<Material>, sp<Bitmap>>>& images)
{
    uint32_t imageSize = 0;
    for(const auto& i : images | std::views::values)
    {
        imageSize = std::max(imageSize, i->width());
        imageSize = std::max(imageSize, i->height());
    }
    imageSize = 1 << (Math::log2(imageSize) + 1);
    return sp<TexturePacker>::make(imageSize, imageSize);
}

}

MaterialBundle::MaterialBundle(const Vector<sp<Material>>& materials)
    : _materials(toMaterialMap(materials)), _stub(sp<Stub>::make())
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

const std::array<sp<Texture>, MaterialTexture::TYPE_LENGTH>& MaterialBundle::textures() const
{
    return _textures;
}

sp<Material> MaterialBundle::getMaterial(const String& name) const
{
    const auto synchronized = _materials.threadSynchronize(_mutex);
    const auto iter = _materials.find(name);
    return iter != _materials.end() ? iter->second : nullptr;
}

void MaterialBundle::addMaterial(String name, sp<Material> material)
{
    const auto synchronized = _materials.threadSynchronize(_mutex);
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
        if(auto images = getMaterialImages(_materials.values(), static_cast<MaterialTexture::Type>(i)); !images.empty())
        {
            texturePackers[i] = makeTexturePackerForImages(images);
            for(auto& [k, v] : images)
                texturePackers[i]->addBitmap(std::move(v), k->name());

            for(const TexturePacker::PackedBitmap& j : texturePackers[i]->packedBitmaps())
                getMaterial(j._name)->setUV(sp<Variable<Rect>>::make<VariableMaterialUV>(_stub, j._uv));

            _stub->_texture_width = std::max(texturePackers[i]->width(), _stub->_texture_width);
            _stub->_texture_height = std::max(texturePackers[i]->height(), _stub->_texture_height);
        }

    const sp<Size> textureSize = sp<Size>::make(_stub->_texture_width, _stub->_texture_height);
    for(size_t i = 0; i < MaterialTexture::TYPE_LENGTH; ++i)
        if(texturePackers[i])
        {
            if(_textures[i])
                texturePackers[i]->updateTexture(_textures[i], textureSize);
            else
                _textures[i] = texturePackers[i]->createTexture(textureSize);
        }
}

}

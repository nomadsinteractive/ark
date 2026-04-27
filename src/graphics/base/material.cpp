#include "graphics/base/material.h"

#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/impl/dictionary/dictionary_by_attribute_name.h"

#include "graphics/base/rect.h"

namespace ark {

Material::Material(const uint32_t id, String name, sp<Bitmap> baseColor, sp<Bitmap> normal, sp<Bitmap> roughness, sp<Bitmap> metallic, sp<Bitmap> specular)
    : _id(id), _name(std::move(name))
{
    _textures[MaterialMap::TYPE_BASE_COLOR] = sp<MaterialMap>::make(nullptr, nullptr, std::move(baseColor));
    _textures[MaterialMap::TYPE_NORMAL] = sp<MaterialMap>::make(nullptr, nullptr, std::move(normal));
    _textures[MaterialMap::TYPE_ROUGHNESS] = sp<MaterialMap>::make(nullptr, nullptr, std::move(roughness));
    _textures[MaterialMap::TYPE_METALLIC] = sp<MaterialMap>::make(nullptr, nullptr, std::move(metallic));
    _textures[MaterialMap::TYPE_SPECULAR] = sp<MaterialMap>::make(nullptr, nullptr, std::move(specular));
    _textures[MaterialMap::TYPE_EMISSION] = sp<MaterialMap>::make();
}

uint32_t Material::id() const
{
    return _id;
}

void Material::setId(const uint32_t id)
{
    _id = id;
}

const String& Material::name() const
{
    return _name;
}

const sp<MaterialMap>& Material::baseColor() const
{
    return _textures[MaterialMap::TYPE_BASE_COLOR];
}

void Material::setBaseColor(sp<MaterialMap> map)
{
    _textures[MaterialMap::TYPE_BASE_COLOR] = std::move(map);
}

const sp<MaterialMap>& Material::normal() const
{
    return _textures[MaterialMap::TYPE_NORMAL];
}

void Material::setNormal(sp<MaterialMap> map)
{
    _textures[MaterialMap::TYPE_NORMAL] = std::move(map);
}

const sp<MaterialMap>& Material::roughness() const
{
    return _textures[MaterialMap::TYPE_ROUGHNESS];
}

void Material::setRoughness(sp<MaterialMap> map)
{
    _textures[MaterialMap::TYPE_ROUGHNESS] = std::move(map);
}

const sp<MaterialMap>& Material::metallic() const
{
    return _textures[MaterialMap::TYPE_METALLIC];
}

void Material::setMetallic(sp<MaterialMap> map)
{
    _textures[MaterialMap::TYPE_METALLIC] = std::move(map);
}

const sp<MaterialMap>& Material::specular() const
{
    return _textures[MaterialMap::TYPE_SPECULAR];
}

void Material::setSpecular(sp<MaterialMap> map)
{
    _textures[MaterialMap::TYPE_SPECULAR] = std::move(map);
}

const sp<MaterialMap>& Material::emission() const
{
    return _textures[MaterialMap::TYPE_EMISSION];
}

void Material::setEmission(sp<MaterialMap> map)
{
    _textures[MaterialMap::TYPE_EMISSION] = std::move(map);
}

const sp<Variable<Rect>>& Material::uv() const
{
    return _uv;
}

void Material::setUV(sp<Variable<Rect>> uv)
{
    _uv = std::move(uv);
}

Rect Material::toTextureUV() const
{
    if(_uv)
        return _uv->val();
    return {0, 1.0f, 1.0f, 0};
}

const sp<MaterialMap>& Material::getTexture(const MaterialMap::Type type) const
{
    ASSERT(type >= 0 && type < MaterialMap::TYPE_LENGTH);
    return _textures[type];
}

}

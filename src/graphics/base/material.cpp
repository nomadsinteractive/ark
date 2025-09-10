#include "graphics/base/material.h"

#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/impl/dictionary/dictionary_by_attribute_name.h"

namespace ark {

Material::Material(const uint32_t id, String name, bitmap baseColor, bitmap normal, bitmap roughness, bitmap metallic, bitmap specular)
    : _id(id), _name(std::move(name))
{
    _textures[MaterialTexture::TYPE_BASE_COLOR] = sp<MaterialTexture>::make("", nullptr, std::move(baseColor));
    _textures[MaterialTexture::TYPE_NORMAL] = sp<MaterialTexture>::make("", nullptr, std::move(normal));
    _textures[MaterialTexture::TYPE_ROUGHNESS] = sp<MaterialTexture>::make("", nullptr, std::move(roughness));
    _textures[MaterialTexture::TYPE_METALLIC] = sp<MaterialTexture>::make("", nullptr, std::move(metallic));
    _textures[MaterialTexture::TYPE_SPECULAR] = sp<MaterialTexture>::make("", nullptr, std::move(specular));
    _textures[MaterialTexture::TYPE_EMISSION] = sp<MaterialTexture>::make("");
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

const sp<MaterialTexture>& Material::baseColor() const
{
    return _textures[MaterialTexture::TYPE_BASE_COLOR];
}

void Material::setBaseColor(sp<MaterialTexture> materialTexture)
{
    _textures[MaterialTexture::TYPE_BASE_COLOR] = std::move(materialTexture);
}

const sp<MaterialTexture>& Material::normal() const
{
    return _textures[MaterialTexture::TYPE_NORMAL];
}

void Material::setNormal(sp<MaterialTexture> materialTexture)
{
    _textures[MaterialTexture::TYPE_NORMAL] = std::move(materialTexture);
}

const sp<MaterialTexture>& Material::roughness() const
{
    return _textures[MaterialTexture::TYPE_ROUGHNESS];
}

void Material::setRoughness(sp<MaterialTexture> materialTexture)
{
    _textures[MaterialTexture::TYPE_ROUGHNESS] = std::move(materialTexture);
}

const sp<MaterialTexture>& Material::metallic() const
{
    return _textures[MaterialTexture::TYPE_METALLIC];
}

void Material::setMetallic(sp<MaterialTexture> materialTexture)
{
    _textures[MaterialTexture::TYPE_METALLIC] = std::move(materialTexture);
}

const sp<MaterialTexture>& Material::specular() const
{
    return _textures[MaterialTexture::TYPE_SPECULAR];
}

void Material::setSpecular(sp<MaterialTexture> materialTexture)
{
    _textures[MaterialTexture::TYPE_SPECULAR] = std::move(materialTexture);
}

const sp<MaterialTexture>& Material::emission() const
{
    return _textures[MaterialTexture::TYPE_EMISSION];
}

void Material::setEmission(sp<MaterialTexture> materialTexture)
{
    _textures[MaterialTexture::TYPE_EMISSION] = std::move(materialTexture);
}

const sp<MaterialTexture>& Material::getTexture(const MaterialTexture::Type type) const
{
    ASSERT(type >= 0 && type < MaterialTexture::TYPE_LENGTH);
    return _textures[type];
}

}

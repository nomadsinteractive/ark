#include "graphics/base/material.h"

#include "core/impl/variable/variable_wrapper.h"
#include "core/impl/dictionary/dictionary_by_attribute_name.h"

namespace ark {

Material::Material(bitmap baseColor, bitmap normal, bitmap roughness, bitmap metallic, bitmap specular)
{
    _textures[TEXTURE_TYPE_BASE_COLOR] = sp<VariableWrapper<bitmap>>::make(std::move(baseColor));
    _textures[TEXTURE_TYPE_NORMAL] = sp<VariableWrapper<bitmap>>::make(std::move(normal));
    _textures[TEXTURE_TYPE_ROUGHNESS] = sp<VariableWrapper<bitmap>>::make(std::move(roughness));
    _textures[TEXTURE_TYPE_METALLIC] = sp<VariableWrapper<bitmap>>::make(std::move(metallic));
    _textures[TEXTURE_TYPE_SPECULAR] = sp<VariableWrapper<bitmap>>::make(std::move(specular));
}

const sp<VariableWrapper<bitmap>>& Material::baseColor() const
{
    return _textures[TEXTURE_TYPE_BASE_COLOR];
}

void Material::setBaseColor(bitmap baseColor) const
{
    _textures[TEXTURE_TYPE_BASE_COLOR]->set(std::move(baseColor));
}

const sp<VariableWrapper<bitmap>>& Material::normal() const
{
    return _textures[TEXTURE_TYPE_NORMAL];
}

void Material::setNormal(bitmap normal) const
{
    _textures[TEXTURE_TYPE_NORMAL]->set(std::move(normal));
}

const sp<VariableWrapper<bitmap>>& Material::roughness() const
{
    return _textures[TEXTURE_TYPE_ROUGHNESS];
}

void Material::setRoughness(bitmap roughness) const
{
    _textures[TEXTURE_TYPE_ROUGHNESS]->set(std::move(roughness));
}

const sp<VariableWrapper<bitmap>>& Material::metallic() const
{
    return _textures[TEXTURE_TYPE_METALLIC];
}

void Material::setMetallic(bitmap metallic) const
{
    _textures[TEXTURE_TYPE_METALLIC]->set(std::move(metallic));
}

const sp<VariableWrapper<bitmap>>& Material::specular() const
{
    return _textures[TEXTURE_TYPE_SPECULAR];
}

void Material::setSpecular(bitmap specular) const
{
    _textures[TEXTURE_TYPE_SPECULAR]->set(std::move(specular));
}

const sp<VariableWrapper<bitmap>>& Material::getTexture(Material::TextureType type) const
{
    DASSERT(type >= 0 && type < TEXTURE_TYPE_LENGTH);
    return _textures[type];
}

Material::BUILDER::BUILDER(BeanFactory& beanFactory, const document& manifest)
{
    DictionaryByAttributeName types(manifest, Constants::Attributes::TYPE);
    _base_color = makeBitmapBuilder(beanFactory, types.get("base_color"));
    _normal = makeBitmapBuilder(beanFactory, types.get("normal"));
    _roughness = makeBitmapBuilder(beanFactory, types.get("roughness"));
    _metallic = makeBitmapBuilder(beanFactory, types.get("metallic"));
    _specular = makeBitmapBuilder(beanFactory, types.get("specular"));
}

sp<Material> Material::BUILDER::build(const Scope& args)
{
    return sp<Material>::make(_base_color->build(args), _normal->build(args), _roughness->build(args), _metallic->build(args), _specular->build(args));
}

sp<Builder<Bitmap>> Material::BUILDER::makeBitmapBuilder(BeanFactory& beanFactory, const document& manifest)
{
    if(manifest)
    {
        const String src = Documents::getAttribute(manifest, Constants::Attributes::SRC);
        if(src)
            return beanFactory.ensureBuilder<Bitmap>(src);

        const String value = Documents::getAttribute(manifest, Constants::Attributes::VALUE);
        DCHECK(value, "You must define a texture by either src or value attribute");
        DFATAL("Unimplemented");
    }
    return nullptr;
}

template<> ARK_API Material::TextureType Conversions::to<String, Material::TextureType>(const String& str)
{
    if(str == "base_color")
        return Material::TEXTURE_TYPE_BASE_COLOR;
    if(str == "normal")
        return Material::TEXTURE_TYPE_NORMAL;
    if(str == "roughness")
        return Material::TEXTURE_TYPE_ROUGHNESS;
    if(str == "metallic")
        return Material::TEXTURE_TYPE_METALLIC;
    DCHECK(str == "specular", "Unknow texture-type: %s, possible values are [base_color, normal, roughness, metallic, specular]");
    return Material::TEXTURE_TYPE_SPECULAR;
}

}

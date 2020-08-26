#include "graphics/base/material.h"

#include "core/impl/variable/variable_wrapper.h"
#include "core/impl/dictionary/dictionary_by_attribute_name.h"

namespace ark {

Material::Material(bitmap baseColor, bitmap normal, bitmap roughness, bitmap metallic, bitmap specular)
    : _base_color(sp<VariableWrapper<bitmap>>::make(std::move(baseColor))), _normal(sp<VariableWrapper<bitmap>>::make(std::move(normal))),
      _roughness(sp<VariableWrapper<bitmap>>::make(std::move(roughness))), _metallic(sp<VariableWrapper<bitmap>>::make(std::move(metallic))),
      _specular(sp<VariableWrapper<bitmap>>::make(std::move(specular)))
{
}

const sp<VariableWrapper<bitmap>>& Material::baseColor() const
{
    return _base_color;
}

void Material::setBaseColor(bitmap baseColor) const
{
    _base_color->set(std::move(baseColor));
}

const sp<VariableWrapper<bitmap>>& Material::normal() const
{
    return _normal;
}

void Material::setNormal(bitmap normal) const
{
    _normal->set(std::move(normal));
}

const sp<VariableWrapper<bitmap>>& Material::roughness() const
{
    return _roughness;
}

void Material::setRoughness(bitmap roughness) const
{
    _roughness->set(std::move(roughness));
}

const sp<VariableWrapper<bitmap>>& Material::metallic() const
{
    return _metallic;
}

void Material::setMetallic(bitmap metallic) const
{
    _metallic->set(std::move(metallic));
}

const sp<VariableWrapper<bitmap>>& Material::specular() const
{
    return _specular;
}

void Material::setSpecular(bitmap specular) const
{
    _specular->set(std::move(specular));
}

Material::BUILDER::BUILDER(BeanFactory& beanFactory, const document& manifest)
{
    DictionaryByAttributeName types(manifest, Constants::Attributes::TYPE);
    _base_color = makeBitmapBuilder(beanFactory, types.get("base-color"));
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

}

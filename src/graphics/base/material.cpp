#include "graphics/base/material.h"

#include "core/base/bean_factory.h"
#include "core/base/constants.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/impl/dictionary/dictionary_by_attribute_name.h"
#include "core/util/documents.h"

namespace ark {

Material::Material(int32_t id, String name, bitmap baseColor, bitmap normal, bitmap roughness, bitmap metallic, bitmap specular)
    : _id(id), _name(std::move(name))
{
    _textures[MaterialTexture::TYPE_BASE_COLOR] = sp<MaterialTexture>::make(nullptr, std::move(baseColor));
    _textures[MaterialTexture::TYPE_NORMAL] = sp<MaterialTexture>::make(nullptr, std::move(normal));
    _textures[MaterialTexture::TYPE_ROUGHNESS] = sp<MaterialTexture>::make(nullptr, std::move(roughness));
    _textures[MaterialTexture::TYPE_METALLIC] = sp<MaterialTexture>::make(nullptr, std::move(metallic));
    _textures[MaterialTexture::TYPE_SPECULAR] = sp<MaterialTexture>::make(nullptr, std::move(specular));
}

int32_t Material::id() const
{
    return _id;
}

const String& Material::name() const
{
    return _name;
}

const sp<MaterialTexture>& Material::baseColor() const
{
    return _textures[MaterialTexture::TYPE_BASE_COLOR];
}

const sp<MaterialTexture>& Material::normal() const
{
    return _textures[MaterialTexture::TYPE_NORMAL];
}

const sp<MaterialTexture>& Material::roughness() const
{
    return _textures[MaterialTexture::TYPE_ROUGHNESS];
}

const sp<MaterialTexture>& Material::metallic() const
{
    return _textures[MaterialTexture::TYPE_METALLIC];
}

const sp<MaterialTexture>& Material::specular() const
{
    return _textures[MaterialTexture::TYPE_SPECULAR];
}

const sp<MaterialTexture>& Material::getTexture(MaterialTexture::Type type) const
{
    ASSERT(type >= 0 && type < MaterialTexture::TYPE_LENGTH);
    return _textures[type];
}

Material::BUILDER::BUILDER(BeanFactory& beanFactory, const document& manifest)
{
    DictionaryByAttributeName types(manifest, constants::TYPE);
    _base_color = makeBitmapBuilder(beanFactory, types.get("base_color"));
    _normal = makeBitmapBuilder(beanFactory, types.get("normal"));
    _roughness = makeBitmapBuilder(beanFactory, types.get("roughness"));
    _metallic = makeBitmapBuilder(beanFactory, types.get("metallic"));
    _specular = makeBitmapBuilder(beanFactory, types.get("specular"));
}

sp<Material> Material::BUILDER::build(const Scope& args)
{
    return sp<Material>::make(0, "", _base_color->build(args), _normal->build(args), _roughness->build(args), _metallic->build(args), _specular->build(args));
}

sp<Builder<Bitmap>> Material::BUILDER::makeBitmapBuilder(BeanFactory& beanFactory, const document& manifest)
{
    if(manifest)
    {
        const String src = Documents::getAttribute(manifest, constants::SRC);
        if(src)
            return beanFactory.ensureBuilder<Bitmap>(src);

        const String value = Documents::getAttribute(manifest, constants::VALUE);
        DCHECK(value, "You must define a texture by either src or value attribute");
        DFATAL("Unimplemented");
    }
    return nullptr;
}

}

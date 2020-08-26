#include "graphics/base/material_bundle.h"

#include "core/base/bean_factory.h"
#include "core/util/documents.h"

namespace ark {

MaterialBundle::MaterialBundle(std::map<String, sp<Material>> materials)
    : _materials(std::move(materials))
{
}

sp<Material> MaterialBundle::getMaterial(const String& name) const
{
    const auto iter = _materials.find(name);
    return iter != _materials.end() ? iter->second : nullptr;
}

MaterialBundle::BUILDER::BUILDER(BeanFactory& beanFactory, const document& manifest)
    : _names(Documents::ensureAttributeList<String>(manifest, "name")), _materials(beanFactory.getBuilderList<Material>(manifest)) {
}

sp<MaterialBundle> MaterialBundle::BUILDER::build(const Scope& args)
{
    std::map<String, sp<Material>> materials;
    DASSERT(_names.size() == _materials.size());
    for(size_t i = 0; i < _names.size(); ++i)
        materials.insert_or_assign(_names.at(i), _materials.at(i)->build(args));
    return sp<MaterialBundle>::make(std::move(materials));
}

}

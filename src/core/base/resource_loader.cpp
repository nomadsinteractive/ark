#include "core/base/resource_loader.h"

#include "core/util/documents.h"
#include "core/util/log.h"

#include "renderer/base/texture_bundle.h"
#include "renderer/base/resource_loader_context.h"

#include "app/base/application_context.h"
#include "app/base/application_manifest.h"

namespace ark {

ResourceLoader::ResourceLoader(const BeanFactory& beanFactory)
    : _bean_factory(beanFactory), _packages(sp<PackageRefs>::make(beanFactory))
{
}

ResourceLoader::~ResourceLoader()
{
    LOGD("");
}

sp<BoxBundle> ResourceLoader::refs() const
{
    return _bean_factory.references();
}

sp<BoxBundle> ResourceLoader::packages() const
{
    return _packages;
}

void ResourceLoader::import(const document& manifest, BeanFactory& parent)
{
    for(const document& i : manifest->children("import"))
    {
        const String name = Documents::getAttribute(i, constants::NAME, Documents::getId(i));
        const String& src = Documents::ensureAttribute(i, constants::SRC);
        const Identifier id(Identifier::parse(src));
        if(id.isRef())
        {
            sp<BeanFactory> package = parent.getPackage(id.ref());
            DCHECK(package, "Package \"%s\" does not exist", src.c_str());
            _bean_factory.addPackage(name, *package);
        }
        else
            _bean_factory.addPackage(name, parent.ensure<ResourceLoader>(src, {})->beanFactory());
    }
}

const BeanFactory& ResourceLoader::beanFactory() const
{
    return _bean_factory;
}

BeanFactory& ResourceLoader::beanFactory()
{
    return _bean_factory;
}

ResourceLoader::BUILDER::BUILDER(BeanFactory& /*factory*/, const document& manifest, const sp<ApplicationContext>& applicationContext)
    : _application_context(applicationContext), _manifest(manifest), _src(Documents::ensureAttribute(manifest, constants::SRC))
{
}

sp<ResourceLoader> ResourceLoader::BUILDER::build(const Scope& /*args*/)
{
    return _application_context->createResourceLoader(_manifest, nullptr);
}

ResourceLoader::DICTIONARY::DICTIONARY(BeanFactory& /*factory*/, const String& value, const sp<ApplicationContext>& applicationContext)
    : _src(value), _application_context(applicationContext)
{
}

sp<ResourceLoader> ResourceLoader::DICTIONARY::build(const Scope& args)
{
    return _application_context->createResourceLoader(_src, args);
}

ResourceLoader::PackageRefs::PackageRefs(const BeanFactory& beanFactory)
    : _bean_factory(beanFactory)
{
}

Box ResourceLoader::PackageRefs::get(const String& name)
{
    const auto iter = _packages.find(name);
    if(iter != _packages.end())
        return Box(iter->second);

    sp<BeanFactory> package = _bean_factory.getPackage(name);
    DCHECK(package, "ResourceLoader has no package named \"%s\"", name.c_str());
    const sp<ResourceLoader> resourceLoader = sp<ResourceLoader>::make(std::move(package));
    _packages.insert(std::make_pair(name, resourceLoader));
    return Box(resourceLoader);
}

}

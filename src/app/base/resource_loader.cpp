#include "app/base/resource_loader.h"

#include "core/util/documents.h"
#include "core/util/log.h"

#include "renderer/base/texture_bundle.h"
#include "renderer/base/resource_loader_context.h"

#include "app/base/application_context.h"

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

sp<BoxBundle> ResourceLoader::layers()
{
    return _builder_refs.ensure<BuilderRefs<Layer>>(_bean_factory);
}

sp<BoxBundle> ResourceLoader::renderLayers()
{
    return _builder_refs.ensure<BuilderRefs<RenderLayer>>(_bean_factory);
}

sp<BoxBundle> ResourceLoader::packages() const
{
    return _packages;
}

void ResourceLoader::import(const document& manifest, BeanFactory& parent)
{
    for(const document& i : manifest->children("import"))
    {
        const String id = Documents::getId(i);
        const String name = Documents::getAttribute(i, Constants::Attributes::NAME, id);
        _bean_factory.addPackage(name, parent.ensure<ResourceLoader>(i, Constants::Attributes::SRC)->beanFactory());
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

ResourceLoader::BUILDER::BUILDER(BeanFactory& factory, const document& doc, const sp<ApplicationContext>& applicationContext)
    : _factory(factory), _application_context(applicationContext), _manifest(doc),
      _src(Documents::ensureAttribute(doc, Constants::Attributes::SRC))
{
}

sp<ResourceLoader> ResourceLoader::BUILDER::build(const sp<Scope>& args)
{
    const sp<ResourceLoader> resourceLoader = _application_context->createResourceLoader(_src, args);
    resourceLoader->import(_manifest, _factory);
    return resourceLoader;
}

ResourceLoader::DICTIONARY::DICTIONARY(BeanFactory& /*factory*/, const String& value, const sp<ApplicationContext>& applicationContext)
    : _src(value), _application_context(applicationContext)
{
}

sp<ResourceLoader> ResourceLoader::DICTIONARY::build(const sp<Scope>& args)
{
    return _application_context->createResourceLoader(_src, nullptr, args);
}

ResourceLoader::PackageRefs::PackageRefs(const BeanFactory& beanFactory)
    : _bean_factory(beanFactory)
{
}

Box ResourceLoader::PackageRefs::get(const String& name)
{
    const auto iter = _packages.find(name);
    if(iter != _packages.end())
        return iter->second.pack();

    const sp<BeanFactory>& package = _bean_factory.getPackage(name);
    DCHECK(package, "ResourceLoader has no package named \"%s\"", name.c_str());
    const sp<ResourceLoader> resourceLoader = sp<ResourceLoader>::make(package);
    _packages.insert(std::make_pair(name, resourceLoader));
    return resourceLoader.pack();
}

}

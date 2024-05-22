#include "resource_loader.h"

#include "core/util/documents.h"
#include "core/util/log.h"
#include "core/util/holder_util.h"

#include "renderer/base/texture_bundle.h"
#include "renderer/base/resource_loader_context.h"

#include "graphics/inf/renderer.h"

#include "app/base/application_context.h"
#include "app/view/arena.h"

namespace ark {

ResourceLoader::ResourceLoader(const BeanFactory& beanFactory)
    : _bean_factory(beanFactory), _packages(sp<PackageRefs>::make(beanFactory))
{
}

ResourceLoader::~ResourceLoader()
{
    LOGD("");
}

void ResourceLoader::traverse(const Holder::Visitor& visitor)
{
    HolderUtil::visit(_bean_factory.references(), visitor);
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
        const String name = Documents::getAttribute(i, Constants::Attributes::NAME, Documents::getId(i));
        const String& src = Documents::ensureAttribute(i, Constants::Attributes::SRC);
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

ResourceLoader::BUILDER::BUILDER(BeanFactory& factory, const document& doc, const sp<ApplicationContext>& applicationContext)
    : _factory(factory), _application_context(applicationContext), _manifest(doc),
      _src(Documents::ensureAttribute(doc, Constants::Attributes::SRC))
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
        return iter->second;

    sp<BeanFactory> package = _bean_factory.getPackage(name);
    DCHECK(package, "ResourceLoader has no package named \"%s\"", name.c_str());
    const sp<ResourceLoader> resourceLoader = sp<ResourceLoader>::make(std::move(package));
    _packages.insert(std::make_pair(name, resourceLoader));
    return resourceLoader;
}

}

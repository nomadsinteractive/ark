#include "core/base/resource_loader.h"

#include "core/util/documents.h"
#include "core/util/log.h"

#include "renderer/base/texture_bundle.h"
#include "renderer/base/resource_loader_context.h"

#include "app/base/application_context.h"
#include "app/base/application_manifest.h"

namespace ark {

ResourceLoader::ResourceLoader(const BeanFactory& beanFactory)
    : _bean_factory(beanFactory)
{
}

ResourceLoader::~ResourceLoader()
{
    LOGD("");
}

void ResourceLoader::import(const document& manifest, BeanFactory& parent)
{
    for(const document& i : manifest->children("import"))
    {
        const String name = Documents::getAttribute(i, constants::NAME, Documents::getId(i));
        const String& src = Documents::ensureAttribute(i, constants::SRC);
        if(const Identifier id(Identifier::parse(src)); id.isRef())
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

}

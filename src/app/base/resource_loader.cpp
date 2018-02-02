#include "app/base/resource_loader.h"

#include "core/util/documents.h"

#include "renderer/base/gl_texture_loader.h"
#include "renderer/base/resource_loader_context.h"

#include "app/base/application_context.h"

namespace ark {

ResourceLoader::ResourceLoader(const BeanFactory& beanFactory)
    : _bean_factory(beanFactory)
{
}

ResourceLoader::~ResourceLoader()
{
    LOGD("");
}

Box ResourceLoader::getReference(const String& name) const
{
    return _bean_factory.references()->get(name);
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

BeanFactory& ResourceLoader::ResourceLoader::beanFactory()
{
    return _bean_factory;
}

ResourceLoader::BUILDER::BUILDER(BeanFactory& parent, const document& doc, const sp<ApplicationContext>& applicationContext)
    : _parent(parent), _application_context(applicationContext), _manifest(doc),
      _src(Documents::ensureAttribute(doc, Constants::Attributes::SRC))
{
}

sp<ResourceLoader> ResourceLoader::BUILDER::build(const sp<Scope>& /*args*/)
{
    const sp<ResourceLoader> resourceLoader = _application_context->createResourceLoader(_src);
    resourceLoader->import(_manifest, _parent);
    return resourceLoader;
}

ResourceLoader::DICTIONARY::DICTIONARY(BeanFactory& /*factory*/, const String& value, const sp<ApplicationContext>& applicationContext)
    : _src(value), _application_context(applicationContext)
{
}

sp<ResourceLoader> ResourceLoader::DICTIONARY::build(const sp<Scope>& /*args*/)
{
    return _application_context->createResourceLoader(_src, nullptr);
}

}

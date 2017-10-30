#include "core/base/plugin.h"

#include "core/types/null.h"

namespace ark {

Plugin::Plugin(const String& name, PluginType type)
    : _name(name), _type(type) {
}

void Plugin::initialize()
{
    _library = createLibrary();
}

BeanFactory::Factory Plugin::createBeanFactory(const BeanFactory& /*beanFactory*/, const sp<Dictionary<document>>& /*documentById*/)
{
    return BeanFactory::Factory();
}

BeanFactory::Factory Plugin::createResourceLoader(const BeanFactory& /*beanFactory*/, const sp<Dictionary<document>>& /*documentById*/, const sp<ResourceLoaderContext>& /*resourceLoaderContext*/)
{
    return BeanFactory::Factory();
}

Library Plugin::createLibrary()
{
    return Library();
}

void Plugin::createScriptModule(const sp<Script>& /*script*/)
{
}

void Plugin::loadBeanFactory(BeanFactory& beanFactory, const sp<Dictionary<document>>& documentById)
{
    BeanFactory::Factory refBeanFactory = createBeanFactory(beanFactory, documentById);
    if(refBeanFactory)
        beanFactory.add(refBeanFactory, _type == Plugin::PLUGIN_TYPE_CORE);
}

void Plugin::loadResourceLoader(BeanFactory& beanFactory, const sp<Dictionary<document>>& documentById, const sp<ResourceLoaderContext>& resourceLoaderContext)
{
    const BeanFactory::Factory resourceLoaderBeanFactory = createResourceLoader(beanFactory, documentById, resourceLoaderContext);
    if(resourceLoaderBeanFactory)
        beanFactory.add(resourceLoaderBeanFactory, _type == Plugin::PLUGIN_TYPE_CORE);
}

const String& Plugin::name() const
{
    return _name;
}

const Library&Plugin::library() const
{
    return _library;
}

}

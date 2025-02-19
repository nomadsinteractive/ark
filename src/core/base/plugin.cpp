#include "core/base/plugin.h"

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
    return {};
}

BeanFactory::Factory Plugin::createResourceLoader(const BeanFactory& /*beanFactory*/, const sp<Dictionary<document>>& /*documentById*/, const sp<ResourceLoaderContext>& /*resourceLoaderContext*/)
{
    return {};
}

Library Plugin::createLibrary()
{
    return {};
}

void Plugin::createScriptModule(Interpreter& /*script*/)
{
}

void Plugin::loadBeanFactory(BeanFactory& beanFactory, const sp<Dictionary<document>>& documentById)
{
    if(BeanFactory::Factory refBeanFactory = createBeanFactory(beanFactory, documentById))
        beanFactory.add(std::move(refBeanFactory), _type == Plugin::PLUGIN_TYPE_CORE);
}

void Plugin::loadResourceLoader(BeanFactory& beanFactory, const sp<Dictionary<document>>& documentById, const sp<ResourceLoaderContext>& resourceLoaderContext)
{
    if(BeanFactory::Factory resourceLoaderBeanFactory = createResourceLoader(beanFactory, documentById, resourceLoaderContext))
        beanFactory.add(std::move(resourceLoaderBeanFactory), _type == Plugin::PLUGIN_TYPE_CORE);
}

const String& Plugin::name() const
{
    return _name;
}

const Library& Plugin::library() const
{
    return _library;
}

}

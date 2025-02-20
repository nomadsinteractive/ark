#include "core/base/plugin.h"

namespace ark {

Plugin::Plugin(const String& name, PluginType type)
    : _name(name), _type(type) {
}

BeanFactory::Factory Plugin::createBeanFactory(const BeanFactory& /*beanFactory*/)
{
    return {};
}

BeanFactory::Factory Plugin::createResourceLoader(const BeanFactory& /*beanFactory*/, const sp<ResourceLoaderContext>& /*resourceLoaderContext*/)
{
    return {};
}

void Plugin::createScriptModule(Interpreter& /*script*/)
{
}

void Plugin::loadBeanFactory(BeanFactory& beanFactory)
{
    if(BeanFactory::Factory refBeanFactory = createBeanFactory(beanFactory))
        beanFactory.add(std::move(refBeanFactory), _type == Plugin::PLUGIN_TYPE_CORE);
}

void Plugin::loadResourceLoader(BeanFactory& beanFactory, const sp<ResourceLoaderContext>& resourceLoaderContext)
{
    if(BeanFactory::Factory resourceLoaderBeanFactory = createResourceLoader(beanFactory, resourceLoaderContext))
        beanFactory.add(std::move(resourceLoaderBeanFactory), _type == Plugin::PLUGIN_TYPE_CORE);
}

const String& Plugin::name() const
{
    return _name;
}

}

#include "core/base/plugin_manager.h"

#include <map>

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/plugin.h"

#include "platform/platform.h"

#ifdef ARK_BUILD_STATIC_PLUGINS
extern std::map<ark::String, ark::PluginManager::PluginInitializer> _ark_static_plugin_initializers;
#endif

namespace ark {

sp<BeanFactory> PluginManager::createBeanFactory(const sp<Dictionary<document>>& documentById) const
{
    const sp<BeanFactory> beanFactory = sp<BeanFactory>::make();
    for(const sp<Plugin>& plugin : _plugins)
        plugin->loadBeanFactory(beanFactory, documentById);
    return beanFactory;
}

void PluginManager::each(const std::function<bool(const sp<Plugin>&)>& visitor) const
{
    for(const sp<Plugin>& plugin : _plugins)
        if(!visitor(plugin))
            break;
}

void PluginManager::load(const String& name)
{
#ifdef ARK_BUILD_STATIC_PLUGINS
    const auto iter = _ark_static_plugin_initializers.find(name);
    PluginInitializer func = reinterpret_cast<PluginInitializer>(iter != _ark_static_plugin_initializers.end() ? iter->second : nullptr);
    DCHECK(func, "Error loading plugin \"%s\"", name.c_str());
#else
    void* library = Platform::dlOpen(name.c_str());
    DCHECK(library, "Cannot load plugin \"%s\"", name.c_str());
    const String symbolName = Strings::sprintf("__%s_initialize__", name.replace("-", "_").c_str());
    void* symbol = Platform::dlSymbol(library, symbolName.c_str());
    PluginInitializer func = reinterpret_cast<PluginInitializer>(symbol);
    DCHECK(func, "Error loading plugin \"%s\", symbol \"%s\" not found", name.c_str(), symbolName.c_str());
#endif
    Plugin* plugin = func(Ark::instance());
    DCHECK(plugin, "Error loading plugin \"%s\", PluginInitializer returned null", name.c_str());
    addPlugin(sp<Plugin>::adopt(plugin));
}

void PluginManager::addPlugin(const sp<Plugin>& plugin)
{
    plugin->initialize();
    _plugins.push_back(plugin);
}

}

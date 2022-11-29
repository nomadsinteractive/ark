#ifndef ARK_CORE_BASE_PLUGIN_MANAGER_H_
#define ARK_CORE_BASE_PLUGIN_MANAGER_H_

#include <functional>
#include <map>
#include <vector>

#include "core/base/api.h"
#include "core/base/plugin.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API PluginManager {
public:

    typedef Plugin* (*PluginInitializer)(Ark&);

    template<typename T> sp<Callable<T>> getCallable(const String& name) const {
        for(const sp<Plugin>& i : _plugins) {
            const sp<Callable<T>> callable = i->library().getCallable<T>(name);
            if(callable)
                return callable;
        }
        return nullptr;
    }

    sp<BeanFactory> createBeanFactory(const sp<Dictionary<document>>& documentById) const;

    void each(const std::function<bool(const sp<Plugin>&)>& visitor) const;

    void load(const String& name);

    void addPlugin(sp<Plugin> plugin);

private:
    std::vector<sp<Plugin>> _plugins;

};

}

#endif

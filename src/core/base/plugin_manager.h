#pragma once

#include <functional>

#include "core/base/api.h"
#include "core/base/plugin.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API PluginManager {
public:

    typedef Plugin* (*PluginInitializer)(Ark&);

    template<typename T> Optional<std::function<T>> getCallable(const String& name) const {
        for(const sp<Plugin>& i : _plugins)
            if(Optional<std::function<T>> callable = i->library().getCallable<T>(name))
                return callable;
        return {};
    }

    sp<BeanFactory> createBeanFactory(const sp<Dictionary<document>>& documentById) const;

    void each(const std::function<bool(const sp<Plugin>&)>& visitor) const;
    void load(const String& name);

    const Vector<sp<Plugin>>& plugins() const;
    void addPlugin(sp<Plugin> plugin);

private:
    Vector<sp<Plugin>> _plugins;
};

}

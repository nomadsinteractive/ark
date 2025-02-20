#pragma once

#include "core/base/api.h"
#include "core/base/plugin.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API PluginManager {
public:

    typedef Plugin* (*PluginInitializer)(Ark&);

    sp<BeanFactory> createBeanFactory(const sp<Dictionary<document>>& documentById) const;

    void load(const String& name);

    const Vector<sp<Plugin>>& plugins() const;
    void addPlugin(sp<Plugin> plugin);

private:
    Vector<sp<Plugin>> _plugins;
};

}

#ifndef ARK_CORE_PLUGIN_PLUGIN_H_
#define ARK_CORE_PLUGIN_PLUGIN_H_

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/base/library.h"
#include "core/collection/by_type.h"
#include "core/types/weak_ptr.h"
#include "core/forwarding.h"

#include "renderer/forwarding.h"

#include "app/base/resource_loader.h"

namespace ark {

class ARK_API Plugin {
public:
    enum PluginType {
        PLUGIN_TYPE_BUILTIN,
        PLUGIN_TYPE_CORE,
        PLUGIN_TYPE_3RDPARTY,
    };

    Plugin(const String& name, PluginType type);
    virtual ~Plugin() = default;

    virtual BeanFactory::Factory createBeanFactory(const BeanFactory& beanFactory, const sp<Dictionary<document>>& documentById);
    virtual BeanFactory::Factory createResourceLoader(const BeanFactory& beanFactory, const sp<Dictionary<document>>& documentById, const sp<ResourceLoaderContext>& resourceLoaderContext);
    virtual Library createLibrary();

    virtual void createScriptModule(const sp<Script>& script);

    void loadBeanFactory(BeanFactory& beanFactory, const sp<Dictionary<document>>& documentById);
    void loadResourceLoader(BeanFactory& beanFactory, const sp<Dictionary<document>>& documentById, const sp<ResourceLoaderContext>& resourceLoaderContext);

    const String& name() const;
    const Library& library() const;

private:
    void initialize();

private:
    Library _library;

    String _name;
    PluginType _type;

    friend class PluginManager;
};

}

#endif

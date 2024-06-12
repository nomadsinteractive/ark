#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/base/library.h"

#include "renderer/forwarding.h"

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

    virtual void createScriptModule(Interpreter& script);

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

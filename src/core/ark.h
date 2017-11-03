#ifndef ARK_CORE_ARK_H_
#define ARK_CORE_ARK_H_

#include <list>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/string.h"
#include "core/collection/by_type.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API Ark {
public:
    enum GLVersion {
        AUTO = 0,
        OPENGL_20 = 20,
        OPENGL_21 = 21,
        OPENGL_30 = 30,
        OPENGL_31 = 31,
        OPENGL_32 = 32,
        OPENGL_33 = 33,
        OPENGL_40 = 40,
        OPENGL_41 = 41,
        OPENGL_42 = 42,
        OPENGL_43 = 43,
        OPENGL_44 = 44,
        OPENGL_45 = 45,
    };

    Ark(int32_t argc, const char** argv, const String& manfiestSrc = String::null());
    ~Ark();

    static Ark& instance();

    void push();

    template<typename T> const sp<T>& query() const {
        return _interfaces.get<T>();
    }

    template<typename T> const sp<T>& global() {
        return _interfaces.ensure<T>();
    }

    template<typename T> void put(const sp<T>& item) {
        _interfaces.put<T>(item);
    }

    sp<BeanFactory> createBeanFactory(const String& src) const;
    sp<BeanFactory> createBeanFactory(const sp<Dictionary<document>>& dictionary) const;

    int32_t argc() const;
    const char** argv() const;

    sp<Asset> getAsset(const String& path) const;
    sp<Readable> getResource(const String& path) const;

    const sp<Clock>& clock() const;
    const sp<ApplicationContext>& applicationContext() const;

private:
    class ArkAsset;

    sp<ApplicationContext> createApplicationContext(const document& manifest);
    sp<RenderEngine> createRenderEngine(GLVersion type);

    void loadPlugins(const document& manifest) const;

private:
    static Ark* _instance;
    static std::list<Ark*> _instance_stack;

    int32_t _argc;
    const char** _argv;

    sp<ApplicationContext> _application_context;
    ByType _interfaces;


    sp<ArkAsset> _asset;
};

}

#endif

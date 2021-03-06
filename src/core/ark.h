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
#include "app/inf/application_profiler.h"

namespace ark {

class ARK_API Ark {
public:
    enum RendererVersion {
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
        OPENGL_46 = 46,
        VULKAN_11 = 111,
        VULKAN_12 = 112
    };

    enum RendererCoordinateSystem {
        COORDINATE_SYSTEM_DEFAULT = 0,
        COORDINATE_SYSTEM_LHS = -1,
        COORDINATE_SYSTEM_RHS = 1
    };

    Ark(int32_t argc, const char** argv);
    Ark(int32_t argc, const char** argv, const sp<Manifest>& manifest);
    ~Ark();

    static Ark& instance();

    void push();
    void initialize(const sp<Manifest>& manifest);

    template<typename T> const sp<T>& query() const {
        return _interfaces.get<T>();
    }

    template<typename T, typename... Args> const sp<T>& ensure(Args&&... args) {
        return _interfaces.ensure<T>(std::forward<Args>(args)...);
    }

    sp<BeanFactory> createBeanFactory(const String& src) const;
    sp<BeanFactory> createBeanFactory(const sp<Dictionary<document>>& dictionary) const;

    int32_t argc() const;
    const char** argv() const;

    const sp<Manifest>& manifest() const;

    sp<AssetBundle> getAssetBundle(const String& path) const;
    sp<Asset> getAsset(const String& path) const;

    sp<Readable> openAsset(const String& path) const;
    sp<Readable> tryOpenAsset(const String& path) const;

    const sp<Clock>& clock() const;
    const sp<ApplicationContext>& applicationContext() const;
    const sp<ApplicationProfiler>& applicationProfiler() const;

    op<ApplicationProfiler::Tracer> makeProfilerTracer(const char* func, const char* filename, int32_t lineno, const char* name, ApplicationProfiler::Category category = ApplicationProfiler::CATEGORY_DEFAULT) const;

    int32_t runTests(int argc, const char* argv[]) const;

private:
    class ArkAssetBundle;

    sp<ApplicationContext> createApplicationContext(const Manifest& manifest, const sp<ApplicationResource>& resource, const sp<RenderEngine>& renderEngine);
    sp<RenderEngine> createRenderEngine(RendererVersion version, RendererCoordinateSystem coordinateSystem, const sp<ApplicationResource>& appResource);

    void loadPlugins(const Manifest& manifest) const;

private:
    static Ark* _instance;
    static std::list<Ark*> _instance_stack;

    int32_t _argc;
    const char** _argv;

    sp<ApplicationContext> _application_context;
    sp<ApplicationProfiler> _application_profiler;
    ByType _interfaces;

    sp<ArkAssetBundle> _asset_bundle;

    sp<Manifest> _manifest;
};

}

#endif

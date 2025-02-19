#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/bit_set.h"
#include "core/base/class_manager.h"
#include "core/base/string.h"
#include "core/collection/traits.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

#include "app/forwarding.h"
#include "app/inf/application_profiler.h"

namespace ark {

class ARK_API Ark {
public:
    enum RenderingBackendBit {
        RENDERING_BACKEND_OPENGL_BIT = 1 << 0,
        RENDERING_BACKEND_VULKAN_BIT = 1 << 1,
        RENDERING_BACKEND_DIRECT_X_BIT = 1 << 2,
        RENDERING_BACKEND_METAL_BIT = 1 << 3,
        RENDERING_BACKEND_ALL = RENDERING_BACKEND_OPENGL_BIT | RENDERING_BACKEND_VULKAN_BIT | RENDERING_BACKEND_DIRECT_X_BIT | RENDERING_BACKEND_METAL_BIT
    };
    typedef BitSet<RenderingBackendBit> RenderingBackendSet;

    enum RendererVersion {
        RENDERER_VERSION_AUTO = 0,
        RENDERER_VERSION_OPENGL_30 = 30,
        RENDERER_VERSION_OPENGL_31 = 31,
        RENDERER_VERSION_OPENGL_32 = 32,
        RENDERER_VERSION_OPENGL_33 = 33,
        RENDERER_VERSION_OPENGL_40 = 40,
        RENDERER_VERSION_OPENGL_41 = 41,
        RENDERER_VERSION_OPENGL_42 = 42,
        RENDERER_VERSION_OPENGL_43 = 43,
        RENDERER_VERSION_OPENGL_44 = 44,
        RENDERER_VERSION_OPENGL_45 = 45,
        RENDERER_VERSION_OPENGL_46 = 46,
        RENDERER_VERSION_VULKAN = 100,
        RENDERER_VERSION_VULKAN_11 = 111,
        RENDERER_VERSION_VULKAN_12 = 112,
        RENDERER_VERSION_VULKAN_13 = 113
    };

    enum RendererCoordinateSystem {
        COORDINATE_SYSTEM_DEFAULT = 0,
        COORDINATE_SYSTEM_LHS = -1,
        COORDINATE_SYSTEM_RHS = 1
    };

    Ark(int32_t argc, const char** argv);
    ~Ark();

    static Ark& instance();

    template<typename T> sp<T> query() const {
        const std::lock_guard lg(_mutex);
        return _interfaces.get<T>();
    }

    template<typename T> sp<T> ensure() {
        const std::lock_guard lg(_mutex);
        return _interfaces.ensure<T>();
    }

    sp<BeanFactory> createBeanFactory(const String& src) const;
    sp<BeanFactory> createBeanFactory(const sp<Dictionary<document>>& dictionary) const;

    int32_t argc() const;
    const char** argv() const;

    void initialize(sp<ApplicationManifest> manifest);
    sp<Application> makeApplication() const;

    const sp<ApplicationManifest>& manifest() const;

    sp<AssetBundle> getAssetBundle(const String& path) const;
    sp<Asset> getAsset(const String& path) const;

    sp<Readable> openAsset(const String& path) const;
    sp<Readable> tryOpenAsset(const String& path) const;

    const sp<Clock>& appClock() const;
    const sp<Clock>& sysClock() const;

    const sp<RenderController>& renderController() const;
    const sp<ApplicationContext>& applicationContext() const;
    const sp<ApplicationProfiler>& applicationProfiler() const;

    Camera createCamera(RendererCoordinateSystem cs = COORDINATE_SYSTEM_DEFAULT) const;
    Camera createCamera(RendererCoordinateSystem cs, bool flip) const;
    Camera createCamera(RendererCoordinateSystem cs, bool flipx, bool flipy) const;

    op<ApplicationProfiler::Tracer> makeProfilerTracer(const char* func, const char* filename, int32_t lineno, const char* name, ApplicationProfiler::Category category = ApplicationProfiler::CATEGORY_DEFAULT) const;
    op<ApplicationProfiler::Logger> makeProfilerLogger(const char* func, const char* filename, int32_t lineno, const char* name) const;

    void deferUnref(Box box) const;

    int32_t runTests();

private:
    void push();

    sp<ApplicationContext> createApplicationContext(const ApplicationManifest& manifest, sp<ApplicationBundle> resource, sp<RenderEngine> renderEngine);

private:
    class ArkAssetBundle;

    int32_t _argc;
    const char** _argv;

    ClassManager _class_manager;
    sp<ApplicationContext> _application_context;
    sp<ApplicationProfiler> _application_profiler;
    sp<ArkAssetBundle> _asset_bundle;
    sp<ApplicationManifest> _manifest;
    Traits _interfaces;

    std::mutex _mutex;
    friend class ClassManager;
};

}

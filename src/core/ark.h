#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/class_manager.h"
#include "core/base/enum.h"
#include "core/base/string.h"
#include "core/collection/traits.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

#include "app/forwarding.h"
#include "app/inf/application_profiler.h"
#include "base/constants.h"

namespace ark {

class ARK_API Ark {
public:
    Ark(int32_t argc, const char** argv);
    ~Ark();

    static Ark& instance();

    template<typename T> sp<T> query() const {
        const auto synchronized = _interfaces.table().threadSynchronize(_mutex);
        return _interfaces.get<T>();
    }

    template<typename T> sp<T> ensure() {
        const auto synchronized = _interfaces.table().threadSynchronize(_mutex);
        return _interfaces.ensure<T>();
    }

    sp<BeanFactory> createBeanFactory(const String& src) const;
    sp<BeanFactory> createBeanFactory(const sp<Dictionary<document>>& dictionary) const;

    int32_t argc() const;
    const char** argv() const;

    void initialize(sp<ApplicationManifest> manifest = nullptr);
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

    const Constants& constants();

    Camera createCamera(enums::CoordinateSystem appCoordinateSystem = enums::COORDINATE_SYSTEM_DEFAULT) const;
    Camera createCamera(enums::CoordinateSystem cs, bool flipx, bool flipy) const;

    op<ApplicationProfiler::Tracer> makeProfilerTracer(const char* func, const char* filename, int32_t lineno, const char* name, ApplicationProfiler::Category category = ApplicationProfiler::CATEGORY_DEFAULT) const;
    op<ApplicationProfiler::Logger> makeProfilerLogger(const char* func, const char* filename, int32_t lineno, const char* name) const;

    void deferUnref(Box box) const;

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

    mutable std::mutex _mutex;
    friend class ClassManager;
};

}

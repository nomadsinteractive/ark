/**
Copyright 2011 - 2024 Jason S Taylor

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
**/

/*
 *  A designer knows he has achieved perfection not when there is nothing left to add,
 *  but when there is nothing left to take away.
 *
 *  Antoine de Saint-Exupery
 *
 */

#include "core/ark.h"


#include "core/base/plugin_manager.h"
#include "core/base/url.h"
#include "core/impl/asset_bundle/asset_bundle_with_fallback.h"
#include "core/impl/asset_bundle/asset_bundle_zip_file.h"
#include "core/impl/dictionary/dictionary_by_attribute_name.h"
#include "core/impl/dictionary/dictionary_impl.h"
#include "core/inf/asset.h"
#include "core/types/global.h"
#include "core/util/asset_bundle_type.h"

#include "renderer/base/render_engine.h"

#ifdef ARK_USE_OPEN_GL
#include "renderer/opengl/renderer_factory/renderer_factory_opengl.h"
#endif

#ifdef ARK_USE_VULKAN
#include "renderer/vulkan/renderer_factory/renderer_factory_vulkan.h"
#endif

#include "app/base/application_bundle.h"
#include "app/base/application_context.h"
#include "app/impl/application/application_delegate_impl.h"
#include "app/base/application_manifest.h"
#include "app/impl/application/sdl_application.h"

#include "platform/platform.h"

#include "generated/ark_bootstrap.h"
#include "generated/base_plugin.h"

namespace ark {

namespace {

Ark* _instance = nullptr;
std::list<Ark*> _instance_stack;

M4 changeProjectionHandSide(const M4& projection, bool flipx, bool flipy, bool flipz)
{
    M4 flip;
    if(flipx)
        flip[0] = -1.0f;
    if(flipy)
        flip[5] = -1.0f;
    if(flipz)
        flip[10] = -1.0f;
    return MatrixUtil::mul(flip, projection);
}

struct CameraDelegateCHS final : Camera::Delegate {

    CameraDelegateCHS(Ark::RendererCoordinateSystem rcs, sp<Delegate> delegate, bool flipx, bool flipy)
        : _rcs(rcs), _delegate(std::move(delegate)), _flipx(flipx), _flipy(flipy)
    {
        ASSERT(_rcs == Ark::COORDINATE_SYSTEM_LHS || _rcs == Ark::COORDINATE_SYSTEM_RHS);
    }

    M4 frustum(float left, float right, float bottom, float top, float clipNear, float clipFar) override
    {
        return changeProjectionHandSide(_delegate->frustum(left, right, bottom, top, clipNear, clipFar), _flipx, _flipy, false);
    }

    M4 lookAt(const V3& position, const V3& target, const V3& up) override
    {
        return _delegate->lookAt(position, target, up);
    }

    M4 ortho(float left, float right, float bottom, float top, float clipNear, float clipFar) override
    {
        const bool needFlipY = (_rcs == Ark::COORDINATE_SYSTEM_LHS ? bottom < top : bottom > top) != _flipy;
        const bool needFlipZ = (_rcs == Ark::COORDINATE_SYSTEM_LHS ? clipNear < clipFar : clipNear > clipFar) != needFlipY;
        const M4 m = _delegate->ortho(left, right, bottom, top, clipNear, clipFar);
        return changeProjectionHandSide(m, false, needFlipY, needFlipZ);
    }

    M4 perspective(float fov, float aspect, float clipNear, float clipFar) override
    {
        return changeProjectionHandSide(_delegate->perspective(fov, aspect, clipNear, clipFar), _flipx, _flipy, false);
    }

    Ark::RendererCoordinateSystem _rcs;
    sp<Delegate> _delegate;
    bool _flipx;
    bool _flipy;
};

sp<RenderEngine> doCreateRenderEngine(const ApplicationManifest::Renderer& renderer, const sp<ApplicationBundle>& applicationBundle)
{
    switch(renderer._version) {
        case Ark::RENDERER_VERSION_AUTO:
        case Ark::RENDERER_VERSION_OPENGL_30:
        case Ark::RENDERER_VERSION_OPENGL_31:
        case Ark::RENDERER_VERSION_OPENGL_32:
        case Ark::RENDERER_VERSION_OPENGL_33:
        case Ark::RENDERER_VERSION_OPENGL_40:
        case Ark::RENDERER_VERSION_OPENGL_41:
        case Ark::RENDERER_VERSION_OPENGL_42:
        case Ark::RENDERER_VERSION_OPENGL_43:
        case Ark::RENDERER_VERSION_OPENGL_44:
        case Ark::RENDERER_VERSION_OPENGL_45:
        case Ark::RENDERER_VERSION_OPENGL_46:
#ifdef ARK_USE_OPEN_GL
            return sp<RenderEngine>::make(renderer, sp<opengl::RendererFactoryOpenGL>::make(applicationBundle->recycler()));
#endif
        case Ark::RENDERER_VERSION_VULKAN:
        case Ark::RENDERER_VERSION_VULKAN_11:
        case Ark::RENDERER_VERSION_VULKAN_12:
        case Ark::RENDERER_VERSION_VULKAN_13:
        {
            ApplicationManifest::Renderer rendererInUse = renderer;
            if(rendererInUse._version == Ark::RENDERER_VERSION_VULKAN)
                rendererInUse._version = Ark::RENDERER_VERSION_VULKAN_13;
#ifdef ARK_USE_VULKAN
            return sp<RenderEngine>::make(rendererInUse, sp<vulkan::RendererFactoryVulkan>::make(applicationBundle->recycler()));
#endif
        }
        break;
    }
    FATAL("Unknown engine type: %d", renderer._version);
    return nullptr;
}

void loadPlugins(const ApplicationManifest& manifest)
{
    const Global<PluginManager> pluginManager;

    for(const String& i : manifest.plugins())
        pluginManager->load(i);
}

sp<RenderEngine> createRenderEngine(BeanFactory& beanFactory, const ApplicationManifest::Renderer& renderer, const sp<ApplicationBundle>& applicationBundle)
{
    if(renderer._class)
        return sp<RenderEngine>::make(renderer, beanFactory.ensure<RendererFactory>(renderer._class, Scope()));

    if(renderer._version != Ark::RENDERER_VERSION_AUTO)
        return doCreateRenderEngine(renderer, applicationBundle);

    ApplicationManifest::Renderer rendererInUse = renderer;
    for(const Ark::RendererVersion i : Platform::getRendererVersionPreferences())
    {
        rendererInUse._version = i;
        if(sp<RenderEngine> renderEngine = doCreateRenderEngine(rendererInUse, applicationBundle))
            return renderEngine;
    }

    FATAL("Cannot create prefered RenderEngine");
    return nullptr;
}

}

class Ark::ArkAssetBundle {
public:
    ArkAssetBundle(const sp<AssetBundle>& builtInAssetBundle, BeanFactory& factory, const std::vector<ApplicationManifest::Asset>& assets)
        : _builtin_asset_bundle(builtInAssetBundle) {
        for(const ApplicationManifest::Asset& i : assets) {
            if(sp<AssetBundle> assetBundle = createAsset(factory, i))
                _mounts.push_front(Mounted(i, assetBundle));
        }
    }

    sp<Asset> getAsset(const String& name) const {
        const URL url(name);
        for(const Mounted& i : _mounts) {
            if(sp<Asset> asset = i.getAsset(url))
                return asset;
        }
        return _builtin_asset_bundle->getAsset(name);
    }

    sp<AssetBundle> getAssetBundle(const String& path) const {
        const URL url(path);
        sp<AssetBundle> asset;

        for(const Mounted& i : _mounts)
            if(const sp<AssetBundle> ia = i.getBundle(url))
                asset = asset ? sp<AssetBundle>::adopt(new AssetBundleWithFallback(asset, ia)) : ia;

        if(const sp<AssetBundle> fallback = _builtin_asset_bundle->getBundle(path))
            return asset ? sp<AssetBundle>::adopt(new AssetBundleWithFallback(asset, fallback)) : fallback;
        CHECK(asset, "AssetBundle \"%s\" doesn't exists", path.c_str());
        return asset;
    }

private:
    sp<AssetBundle> createAsset(BeanFactory& factory, const ApplicationManifest::Asset& manifest) const
    {
        sp<AssetBundle> asset = manifest._protocol.empty() ? _builtin_asset_bundle->getBundle(manifest._src) :
                                                             factory.build<AssetBundle>(manifest._protocol, manifest._src, {});
        CHECK_WARN(asset, "Unable to load AssetBundle, protocol: %s, src: %s", manifest._protocol.c_str(), manifest._src.c_str());
        return asset;
    }

    class Mounted {
    public:
        Mounted(const ApplicationManifest::Asset& manifest, sp<AssetBundle> asset)
            : _root(manifest._protocol, manifest._root), _asset_bundle(std::move(asset)) {
        }

        sp<Asset> getAsset(const URL& url) const {
            String relpath;
            if(getRelativePath(url, relpath))
                return _asset_bundle->getAsset(relpath);
            return nullptr;
        }

        sp<AssetBundle> getBundle(const URL& url) const {
            String relpath;
            if(getRelativePath(url, relpath)) {
                if(relpath.empty())
                    return _asset_bundle;
                const sp<AssetBundle> asset = _asset_bundle->getBundle(relpath);
                if(asset)
                    return asset;

                const String filename = relpath.rstrip('/');
                const sp<Asset> fp = _asset_bundle->getAsset(filename);
                if(fp)
                    return sp<AssetBundleZipFile>::make(fp->open(), Platform::getRealPath(filename));
            }
            return nullptr;
        }

    private:
        bool getRelativePath(const URL& url, String& relpath) const {
            if(url.protocol() == _root.protocol() || _root.path().empty()) {
                relpath = url.path();
                return true;
            }
            if(url.protocol().empty() && url.path().startsWith(_root.path())) {
                relpath = url.path().substr(_root.path().length());
                return true;
            }
            return false;
        }

    private:
        URL _root;
        sp<AssetBundle> _asset_bundle;
    };

    std::list<Mounted> _mounts;
    sp<AssetBundle> _builtin_asset_bundle;
};

Ark::Ark(int32_t argc, const char** argv)
    : _argc(argc), _argv(argv)
{
    push();
    __ark_bootstrap__();
}

Ark::~Ark()
{
    for(auto iter = _instance_stack.begin(); iter != _instance_stack.end(); ++iter)
        if(*iter == this)
        {
            iter = _instance_stack.erase(iter);
            if(iter == _instance_stack.end())
                break;
        }
    if(_instance == this)
        _instance = _instance_stack.size() > 0 ? _instance_stack.front() : nullptr;
}

Ark& Ark::instance()
{
    DCHECK(_instance, "Create an Ark instance before using the others");
    return *_instance;
}

void Ark::push()
{
    _instance_stack.push_front(_instance);
    _instance = this;
}

void Ark::initialize(sp<ApplicationManifest> manifest)
{
    _manifest = std::move(manifest);

    loadPlugins(_manifest);

    sp<BeanFactory> factory = createBeanFactory(sp<DictionaryImpl<document>>::make());
    _asset_bundle = sp<ArkAssetBundle>::make(AssetBundleType::createBuiltInAssetBundle(_manifest->assetDir(), _manifest->appDir()), factory, _manifest->assets());
    sp<ApplicationBundle> applicationBundle = sp<ApplicationBundle>::make(_asset_bundle->getAssetBundle("/"));
    sp<RenderEngine> renderEngine = createRenderEngine(factory, _manifest->renderer(), applicationBundle);
    _application_context = createApplicationContext(_manifest, std::move(applicationBundle), std::move(renderEngine));
}

sp<BeanFactory> Ark::createBeanFactory(const String& src) const
{
    const Global<Dictionary<document>> resources;
    const document doc = resources->get(src);
    CHECK(doc, "Resource \"%s\" not found", src.c_str());
    return createBeanFactory(sp<DictionaryByAttributeName>::make(doc, constants::ID));
}

sp<BeanFactory> Ark::createBeanFactory(const sp<Dictionary<document>>& dictionary) const
{
    const Global<PluginManager> pluginManager;
    return pluginManager->createBeanFactory(dictionary);
}

int32_t Ark::argc() const
{
    return _argc;
}

const char** Ark::argv() const
{
    return _argv;
}

sp<Application> Ark::makeApplication(sp<ApplicationManifest> manifest, uint32_t width, uint32_t height)
{
    initialize(std::move(manifest));
    return sp<SDLApplication>::make<SDLApplication>(sp<ApplicationDelegateImpl>::make(_manifest), _application_context, width, height, _manifest);
}

const sp<ApplicationManifest>& Ark::manifest() const
{
    return _manifest;
}

sp<AssetBundle> Ark::getAssetBundle(const String& path) const
{
    return _asset_bundle->getAssetBundle(path);
}

sp<Asset> Ark::getAsset(const String& path) const
{
    return _asset_bundle->getAsset(path);
}

sp<Readable> Ark::openAsset(const String& path) const
{
    const sp<Asset> asset = _asset_bundle->getAsset(path);
    CHECK(asset, "Cannot open asset \"%s\"", path.c_str());
    return asset->open();
}

sp<Readable> Ark::tryOpenAsset(const String& path) const
{
    const sp<Asset> asset = _asset_bundle->getAsset(path);
    return asset ? asset->open() : sp<Readable>();
}

const sp<Clock>& Ark::appClock() const
{
    return _application_context->appClock();
}

const sp<Clock>& Ark::sysClock() const
{
    return _application_context->sysClock();
}

const sp<RenderController>& Ark::renderController() const
{
    return _application_context->renderController();
}

const sp<ApplicationContext>& Ark::applicationContext() const
{
    return _application_context;
}

const sp<ApplicationProfiler>& Ark::applicationProfiler() const
{
    return _application_profiler;
}

Camera Ark::createCamera(RendererCoordinateSystem coordinateSystem, bool flipx) const
{
    if(coordinateSystem == COORDINATE_SYSTEM_DEFAULT)
        coordinateSystem = _manifest->renderer()._coordinate_system;

    RendererFactory& rendererFactory = _application_context->renderController()->renderEngine()->rendererFactory();
    const RendererCoordinateSystem cs = coordinateSystem == COORDINATE_SYSTEM_DEFAULT ? rendererFactory.features()._default_coordinate_system : coordinateSystem;
    sp<Camera::Delegate> cameraDelegate = rendererFactory.createCamera();
    const bool flipy = cs != rendererFactory.features()._default_coordinate_system;
    if(flipx || flipy)
        return {cs, sp<Camera::Delegate>::make<CameraDelegateCHS>(cs, std::move(cameraDelegate), flipx, flipy)};
    return {cs, std::move(cameraDelegate)};
}

op<ApplicationProfiler::Tracer> Ark::makeProfilerTracer(const char* func, const char* filename, int32_t lineno, const char* name, ApplicationProfiler::Category category) const
{
    return _application_profiler ? _application_profiler->makeTracer(func, filename, lineno, name, category) : op<ApplicationProfiler::Tracer>();
}

op<ApplicationProfiler::Logger> Ark::makeProfilerLogger(const char* func, const char* filename, int32_t lineno, const char* name) const
{
    return _application_profiler ? _application_profiler->makeLogger(func, filename, lineno, name) : op<ApplicationProfiler::Logger>();
}

void Ark::deferUnref(Box box) const
{
    _application_context->renderController()->deferUnref(std::move(box));
}

sp<ApplicationContext> Ark::createApplicationContext(const ApplicationManifest& manifest, sp<ApplicationBundle> appResource, sp<RenderEngine> renderEngine)
{
    const Global<PluginManager> pluginManager;
    const sp<ApplicationContext> applicationContext = sp<ApplicationContext>::make(std::move(appResource), std::move(renderEngine));
    pluginManager->addPlugin(sp<BasePlugin>::make(applicationContext));
    applicationContext->initialize(manifest.resourceLoader());
    _application_profiler = applicationContext->resourceLoader()->beanFactory().build<ApplicationProfiler>(document::make("root"), Scope());
    return applicationContext;
}

}

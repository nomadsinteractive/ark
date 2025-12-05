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
#include "core/impl/asset_bundle/asset_bundle_compound.h"
#include "core/impl/asset_bundle/asset_bundle_zip_file.h"
#include "core/impl/dictionary/dictionary_by_attribute_name.h"
#include "core/impl/dictionary/dictionary_impl.h"
#include "core/inf/asset.h"
#include "core/types/global.h"
#include "core/types/optional.h"
#include "core/util/asset_bundle_type.h"

#include "renderer/base/render_engine.h"
#include "renderer/inf/renderer_factory.h"

#include "app/base/application_bundle.h"
#include "app/base/application_context.h"
#include "app/base/application_manifest.h"

#include "platform/platform.h"

#include "generated/ark_bootstrap.h"
#include "generated/base_plugin.h"

namespace ark {

namespace {

Ark* _instance = nullptr;
List<Ark*> _instance_stack;

M4 changeProjectionHandSide(const M4& projection, const bool flipx, const bool flipy, const bool flipz)
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

    CameraDelegateCHS(const enums::CoordinateSystem rcs, sp<Delegate> delegate, const bool flipx, const bool flipy)
        : _rcs(rcs), _delegate(std::move(delegate)), _flipx(flipx), _flipy(flipy)
    {
        ASSERT(_rcs == enums::COORDINATE_SYSTEM_LHS || _rcs == enums::COORDINATE_SYSTEM_RHS);
    }

    M4 frustum(const float left, const float right, const float bottom, const float top, const float clipNear, const float clipFar) override
    {
        return changeProjectionHandSide(_delegate->frustum(left, right, bottom, top, clipNear, clipFar), _flipx, _flipy, false);
    }

    M4 lookAt(const V3& position, const V3& target, const V3& up) override
    {
        return _delegate->lookAt(position, target, up);
    }

    M4 ortho(const float left, const float right, const float bottom, const float top, const float clipNear, const float clipFar) override
    {
        const bool needFlipY = (_rcs == enums::COORDINATE_SYSTEM_LHS ? bottom < top : bottom > top) != _flipy;
        const bool needFlipZ = (_rcs == enums::COORDINATE_SYSTEM_LHS ? clipNear < clipFar : clipNear > clipFar) != needFlipY;
        const M4 m = _delegate->ortho(left, right, bottom, top, clipNear, clipFar);
        return changeProjectionHandSide(m, false, needFlipY, needFlipZ);
    }

    M4 perspective(const float fov, const float aspect, const float clipNear, const float clipFar) override
    {
        return changeProjectionHandSide(_delegate->perspective(fov, aspect, clipNear, clipFar), _flipx, _flipy, false);
    }

    enums::CoordinateSystem _rcs;
    sp<Delegate> _delegate;
    bool _flipx;
    bool _flipy;
};

sp<RendererFactory> chooseRenderFactory(const Vector<sp<RendererFactory>>& rendererFactories, const enums::RenderingBackendBit renderingBackend)
{
    for(const sp<RendererFactory>& i : rendererFactories)
        if(i->features()._supported_backends.contains(renderingBackend))
            return i;
    return nullptr;
}

sp<RenderEngine> doCreateRenderEngine(BeanFactory& beanFactory, const ApplicationManifest::Renderer& renderer)
{
    Vector<sp<RendererFactory>> rendererFactories;
    for(const auto& [k, v] : beanFactory.makeValueBuilderList<RendererFactory>())
        rendererFactories.emplace_back(v->build({}));

    ApplicationManifest::Renderer rendererInUse = renderer;
    if(rendererInUse._version == enums::RENDERER_VERSION_OPENGL)
        rendererInUse._version = enums::RENDERER_VERSION_OPENGL_46;
    else if(rendererInUse._version == enums::RENDERER_VERSION_VULKAN)
        rendererInUse._version = enums::RENDERER_VERSION_VULKAN_13;

    if(sp<RendererFactory> rendererFactory = chooseRenderFactory(rendererFactories, rendererInUse._backend))
        return sp<RenderEngine>::make(rendererInUse, std::move(rendererFactory));
    return nullptr;
}

void loadPlugins(const ApplicationManifest& manifest)
{
    const Global<PluginManager> pluginManager;

    for(const String& i : manifest.plugins())
        pluginManager->load(i);
}

sp<RenderEngine> createRenderEngine(BeanFactory& beanFactory, const ApplicationManifest::Renderer& renderer)
{
    if(renderer._class)
        return sp<RenderEngine>::make(renderer, beanFactory.ensure<RendererFactory>(renderer._class, {}));

    if(renderer._version != enums::RENDERER_VERSION_AUTO)
        return doCreateRenderEngine(beanFactory, renderer);

    ApplicationManifest::Renderer rendererInUse = renderer;
    for(const enums::RenderingBackendBit i : Platform::getPreferedRenderBackends())
    {
        rendererInUse._backend = i;
        if(sp<RenderEngine> renderEngine = doCreateRenderEngine(beanFactory, rendererInUse))
            return renderEngine;
    }

    FATAL("Cannot create prefered RenderEngine");
    return nullptr;
}

String getMountedPath(const String& root)
{
    if(root.empty() || root.endsWith("/"))
        return root;

    return root + "/";
}

sp<AssetBundle> createAssetBundle(const ApplicationManifest::Asset& asset)
{
    const String filepath = asset._src.protocol() == "external" ? Platform::getExternalStoragePath(asset._src.path()) : asset._src.path();
    sp<AssetBundle> assetBundle = Platform::getAssetBundle(filepath);
    CHECK_WARN(assetBundle, "Unable to load AssetBundle, src: %s", asset._src.toString().c_str());
    return assetBundle;
}

}

class Ark::ArkAssetBundle {
public:
    ArkAssetBundle(sp<AssetBundle> defaultAssetBundle, const Vector<ApplicationManifest::Asset>& assets)
        : _default_asset_bundle(std::move(defaultAssetBundle))
    {
        for(const ApplicationManifest::Asset& i : assets)
            if(sp<AssetBundle> assetBundle = createAssetBundle(i))
                _mounts.push_front(Mounted(i, std::move(assetBundle)));
    }

    sp<Asset> getAsset(const String& name) const
    {
        const URL url(name);
        for(const Mounted& i : _mounts)
        {
            if(sp<Asset> asset = i.getAsset(url))
                return asset;
        }
        return _default_asset_bundle->getAsset(name);
    }

    sp<AssetBundle> getAssetBundle(const String& path) const
    {
        const URL url(path);
        sp<AssetBundle> asset;

        for(const Mounted& i : _mounts)
            if(sp<AssetBundle> ia = i.getBundle(url))
                asset = asset ? sp<AssetBundle>::make<AssetBundleCompound>(std::move(asset), std::move(ia)) : std::move(ia);

        if(sp<AssetBundle> fallback = _default_asset_bundle->getBundle(path))
            return asset ? sp<AssetBundle>::make<AssetBundleCompound>(std::move(asset), std::move(fallback)) : std::move(fallback);
        CHECK(asset, "AssetBundle \"%s\" doesn't exists", path.c_str());
        return asset;
    }

private:

    class Mounted {
    public:
        Mounted(const ApplicationManifest::Asset& manifest, sp<AssetBundle> assetBundle)
            : _root(manifest._src.protocol(), getMountedPath(manifest._root)), _asset_bundle(std::move(assetBundle)) {
        }

        sp<Asset> getAsset(const URL& url) const
        {
            if(const Optional<String> relPath = getRelativePath(url))
                return _asset_bundle->getAsset(relPath.value());

            return nullptr;
        }

        sp<AssetBundle> getBundle(const URL& url) const
        {
            if(const Optional<String> relPath = getRelativePath(url))
            {
                if(relPath->empty())
                    return _asset_bundle;

                if(sp<AssetBundle> asset = _asset_bundle->getBundle(relPath.value()))
                    return asset;

                const String filename = relPath->rstrip('/');
                if(const sp<Asset> fp = _asset_bundle->getAsset(filename))
                    return sp<AssetBundle>::make<AssetBundleZipFile>(fp->open(), Platform::getRealPath(filename));
            }
            return nullptr;
        }

    private:
        Optional<String> getRelativePath(const URL& url) const {
            if(url.protocol() != _root.protocol())
                return {};

            if(_root.path().empty())
                return {url.path()};

            if(url.path() == _root.path().substr(0, _root.path().length() - 1))
                return {""};

            if(url.path().startsWith(_root.path()))
                return {url.path().substr(_root.path().length())};

            return {};
        }

    private:
        URL _root;
        sp<AssetBundle> _asset_bundle;
    };

    List<Mounted> _mounts;
    sp<AssetBundle> _default_asset_bundle;
};

Ark::Ark(const int32_t argc, const char** argv)
    : _argc(argc), _argv(argv)
{
    push();
    __ark_bootstrap__();
}

Ark::~Ark()
{
    _application_context->finalize();
    _application_context = nullptr;
    _interfaces = {};

    for(auto iter = _instance_stack.begin(); iter != _instance_stack.end(); )
        if(*iter == this)
            iter = _instance_stack.erase(iter);
        else
            ++iter;

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

    _asset_bundle = sp<ArkAssetBundle>::make(AssetBundleType::createBuiltInAssetBundle(), _manifest->assets());
    sp<ApplicationBundle> applicationBundle = sp<ApplicationBundle>::make(_asset_bundle->getAssetBundle("/"));

    const sp<BeanFactory> factory = createBeanFactory(sp<Dictionary<document>>::make<DictionaryImpl<document>>());
    sp<RenderEngine> renderEngine = createRenderEngine(factory, _manifest->renderer());
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

sp<Application> Ark::makeApplication() const
{
    sp<Application> application = _application_context->resourceLoader()->beanFactory().build<Application>(Global<Constants>()->DOCUMENT_NONE, {});
    CHECK(application, "Failed to create application. It's likely certain plugins are not loaded, such as SDL.");
    return application;
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

const Constants& Ark::constants()
{
    return ensure<Constants>();
}

Camera Ark::createCamera(enums::CoordinateSystem cs) const
{
    if(cs == enums::COORDINATE_SYSTEM_DEFAULT)
        cs = _manifest->renderer()._coordinate_system;
    if(cs == enums::COORDINATE_SYSTEM_DEFAULT)
        cs = _application_context->renderController()->renderEngine()->viewportCoordinateSystem();
    return createCamera(cs, false, false);
}

Camera Ark::createCamera(enums::CoordinateSystem cs, bool flipx, bool flipy) const
{
    RendererFactory& rendererFactory = _application_context->renderController()->renderEngine()->rendererFactory();
    sp<Camera::Delegate> cameraDelegate = rendererFactory.createCamera(cs);
    if(flipx || flipy)
        return {cs, sp<Camera::Delegate>::make<CameraDelegateCHS>(cs, std::move(cameraDelegate), flipx, flipy)};
    return {cs, std::move(cameraDelegate)};
}

op<ApplicationProfiler::Tracer> Ark::makeProfilerTracer(const char* func, const char* filename, const int32_t lineno, const char* name, const ApplicationProfiler::Category category) const
{
    return _application_profiler ? _application_profiler->makeTracer(func, filename, lineno, name, category) : op<ApplicationProfiler::Tracer>();
}

op<ApplicationProfiler::Logger> Ark::makeProfilerLogger(const char* func, const char* filename, const int32_t lineno, const char* name) const
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

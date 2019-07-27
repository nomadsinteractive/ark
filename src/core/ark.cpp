/**
Copyright 2011 - 2019 Jason S Taylor

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

#include "core/base/manifest.h"
#include "core/base/plugin_manager.h"
#include "core/base/url.h"
#include "core/impl/asset_bundle/asset_bundle_with_fallback.h"
#include "core/impl/asset_bundle/asset_bundle_zip_file.h"
#include "core/impl/dictionary/dictionary_by_attribute_name.h"
#include "core/impl/dictionary/dictionary_impl.h"
#include "core/impl/dictionary/xml_directory.h"
#include "core/inf/asset.h"
#include "core/types/global.h"
#include "core/util/asset_bundle_util.h"

#include "renderer/base/render_engine.h"

#ifdef ARK_USE_OPEN_GL
#include "renderer/opengl/renderer_factory/renderer_factory_opengl.h"
#endif

#ifdef ARK_USE_VULKAN
#include "renderer/vulkan/renderer_factory/renderer_factory_vulkan.h"
#endif

#include "app/base/application_context.h"
#include "app/base/application_resource.h"

#include "platform/platform.h"

#include "generated/ark_bootstrap.h"
#include "generated/application_plugin.h"
#include "generated/framework_plugin.h"

#include "platform/platform.h"

namespace ark {

Ark* Ark::_instance = nullptr;
std::list<Ark*> Ark::_instance_stack;


class Ark::ArkAssetBundle {
public:
    ArkAssetBundle(const sp<AssetBundle>& builtInAssetBundle, BeanFactory& factory, const std::vector<Manifest::Asset>& assets)
        : _builtin_asset_bundle(builtInAssetBundle) {
        for(const Manifest::Asset& i : assets) {
            sp<AssetBundle> assetBundle = createAsset(factory, i);
            if(assetBundle)
                _mounts.push_front(Mounted(i, assetBundle));
        }
    }

    sp<Asset> get(const String& name) const {
        const URL url(name);

        for(const Mounted& i : _mounts) {
            const sp<Asset> readable = i.open(url);
            if(readable)
                return readable;
        }
        return _builtin_asset_bundle->get(name);
    }

    sp<AssetBundle> getAssetBundle(const String& path) const {
        const URL url(path);
        sp<AssetBundle> asset;

        for(const Mounted& i : _mounts) {
            const sp<AssetBundle> ia = i.getBundle(url);
            if(ia)
                asset = asset ? sp<AssetBundle>::adopt(new AssetBundleWithFallback(asset, ia)) : ia;
        }
        const sp<AssetBundle> fallback = _builtin_asset_bundle->getBundle(path);
        if(fallback)
            return asset ? sp<AssetBundle>::adopt(new AssetBundleWithFallback(asset, fallback)) : fallback;
        DCHECK(asset, "AssetBundle \"%s\" doesn't exists", path.c_str());
        return asset;
    }

private:
    sp<AssetBundle> createAsset(BeanFactory& factory, const Manifest::Asset& manifest) {
        sp<AssetBundle> asset = manifest._protocol.empty() ? _builtin_asset_bundle->getBundle(manifest._src) :
                                                             factory.build<AssetBundle>(manifest._protocol, manifest._src);
        DWARN(asset, "Unable to load AssetBundle, protocol: %s, src: %s", manifest._protocol.c_str(), manifest._src.c_str());
        return asset;
    }

    class Mounted {
    public:
        Mounted(const Manifest::Asset& manifest, const sp<AssetBundle>& asset)
            : _root(manifest._protocol, manifest._root), _asset_bundle(asset) {
        }

        sp<Asset> open(const URL& url) const {
            String relpath;
            if(getRelativePath(url, relpath))
                return _asset_bundle->get(relpath);
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
                const sp<Asset> fp = _asset_bundle->get(filename);
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
    : _argc(argc), _argv(argv), _object_pool(sp<ObjectPool>::make())
{
    push();
    __ark_bootstrap__();
}

Ark::Ark(int32_t argc, const char** argv, const sp<Manifest>& manifest)
    : Ark(argc, argv)
{
    initialize(manifest);
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

void Ark::initialize(const sp<Manifest>& manifest)
{
    _manifest = manifest;

    loadPlugins(_manifest);

    const sp<BeanFactory> factory = createBeanFactory(sp<DictionaryImpl<document>>::make());
    _asset_bundle = sp<ArkAssetBundle>::make(AssetBundleUtil::createBuiltInAssetBundle(_manifest->assetDir(), _manifest->appDir()), factory, _manifest->assets());
    const sp<AssetBundle> asset = _asset_bundle->getAssetBundle("/");
    const sp<ApplicationResource> appResource = sp<ApplicationResource>::make(sp<XMLDirectory>::make(asset), asset);
    const sp<RenderEngine> renderEngine = createRenderEngine(_manifest->renderer()._version, appResource);
    _application_context = createApplicationContext(_manifest, appResource, renderEngine);
}

sp<BeanFactory> Ark::createBeanFactory(const String& src) const
{
    const Global<Dictionary<document>> resources;
    const document doc = resources->get(src);
    DCHECK(doc, "Resource \"%s\" not found", src.c_str());
    return createBeanFactory(sp<DictionaryByAttributeName>::make(doc, Constants::Attributes::ID));
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

const sp<Manifest>& Ark::manifest() const
{
    return _manifest;
}

sp<AssetBundle> Ark::getAssetBundle(const String& path) const
{
    return _asset_bundle->getAssetBundle(path);
}

sp<Asset> Ark::getAsset(const String& path) const
{
    return _asset_bundle->get(path);
}

sp<Readable> Ark::openAsset(const String& path) const
{
    const sp<Asset> asset = _asset_bundle->get(path);
    DCHECK(asset, "Cannot open asset \"%s\"", path.c_str());
    return asset->open();
}

sp<Readable> Ark::tryOpenAsset(const String& path) const
{
    const sp<Asset> asset = _asset_bundle->get(path);
    return asset ? asset->open() : sp<Readable>::null();
}

const sp<Clock>& Ark::clock() const
{
    return _application_context->clock();
}

const sp<ApplicationContext>& Ark::applicationContext() const
{
    return _application_context;
}

const sp<ObjectPool>& Ark::objectPool() const
{
    return _object_pool;
}

sp<ApplicationContext> Ark::createApplicationContext(const Manifest& manifest, const sp<ApplicationResource>& appResource, const sp<RenderEngine>& renderEngine)
{
    const Global<PluginManager> pluginManager;
    const sp<ApplicationContext> applicationContext = sp<ApplicationContext>::make(appResource, renderEngine);
    pluginManager->addPlugin(sp<ApplicationPlugin>::make(applicationContext));
    applicationContext->initResourceLoader(manifest.resourceLoader());
    return applicationContext;
}

sp<RenderEngine> Ark::createRenderEngine(RendererVersion version, const sp<ApplicationResource>& appResource)
{
    switch(version) {
    case AUTO:
    case OPENGL_20:
    case OPENGL_21:
    case OPENGL_30:
    case OPENGL_31:
    case OPENGL_32:
    case OPENGL_33:
    case OPENGL_40:
    case OPENGL_41:
    case OPENGL_42:
    case OPENGL_43:
    case OPENGL_44:
    case OPENGL_45:
    case OPENGL_46:
#ifdef ARK_USE_OPEN_GL
        return sp<RenderEngine>::make(version, sp<opengl::RendererFactoryOpenGL>::make(appResource->recycler()));
#endif
    case VULKAN_11:
#ifdef ARK_USE_VULKAN
        return sp<RenderEngine>::make(version, sp<vulkan::RendererFactoryVulkan>::make(appResource->recycler()));
#endif
    }
    DFATAL("Unknown engine type: %d", version);
    return nullptr;
}

void Ark::loadPlugins(const Manifest& manifest) const
{
    const Global<PluginManager> pluginManager;

    for(const String& i : manifest.plugins())
        pluginManager->load(i);
}

}

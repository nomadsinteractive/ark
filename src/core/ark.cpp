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
#include "core/impl/asset_bundle/asset_bundle_with_fallback.h"
#include "core/impl/asset_bundle/asset_bundle_with_prefix.h"
#include "core/impl/asset_bundle/asset_bundle_directory.h"
#include "core/impl/asset_bundle/asset_bundle_zip_file.h"
#include "core/impl/readable/file_readable.h"
#include "core/impl/dictionary/dictionary_by_attribute_name.h"
#include "core/impl/dictionary/xml_directory.h"
#include "core/inf/asset.h"
#include "core/types/global.h"

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

namespace {

class RawAssetBundle : public AssetBundle {
public:
    RawAssetBundle(const String& assetDir, const String& appDir)
        : _asset_dir(assetDir), _app_dir(appDir) {
    }

    virtual sp<Asset> get(const String& filepath) override {
        String dirname, filename;
        Strings::rcut(filepath, dirname, filename, '/');
        const sp<AssetBundle> dir = getBundle(dirname);
        const sp<Asset> asset = dir ? dir->get(filename) : nullptr;
        LOGD("filepath(%s) dirname(%s) ==> dir<%p> asset<%p>", filepath.c_str(), dirname.c_str(), dir.get(), asset.get());
        return asset;
    }

    virtual sp<AssetBundle> getBundle(const String& path) override {
        String s = (path.empty() || path == "/") ? "." : path;
        const String assetDir = Platform::pathJoin(_asset_dir, s);
        const sp<AssetBundle> assetBundle = Platform::getAssetBundle(s, Platform::pathJoin(_app_dir, assetDir));
        if(assetBundle)
            return assetBundle;

        const sp<Asset> asset = get(path);
        if(asset)
            return sp<AssetBundleZipFile>::make(asset->open(), path);

        String dirname;
        String filename;

        do {
            String name;
            Strings::rcut(s, dirname, name, '/');
            filename = filename.empty() ? name : name + "/" + filename;
            const sp<Asset> asset = dirname.empty() ? nullptr : get(dirname);
            if(asset) {
                const sp<AssetBundleZipFile> zip = sp<AssetBundleZipFile>::make(asset->open(), dirname);
                const String entryName = filename + "/";
                return zip->hasEntry(entryName) ? sp<AssetBundleWithPrefix>::make(zip, entryName) : nullptr;
            }
            s = dirname;
        } while(!dirname.empty());
        return nullptr;
    }

    String _asset_dir;
    String _app_dir;
};

}


class Ark::ArkAssetBundle {
public:
    ArkAssetBundle(const sp<RawAssetBundle>& rawAsset, const Table<String, String>& mounts)
        : _raw_asset_bundle(rawAsset) {
        for(const auto& i : mounts.items())
            _mounts.push_front(Mounted(strip(i.first), createAsset(i.second)));
    }

    sp<Asset> get(const String& name) const {
        String pathname = name;
        while(pathname.startsWith("/") || pathname.startsWith("."))
            pathname = pathname.substr(1);

        for(const Mounted& i : _mounts) {
            const sp<Asset> readable = i.open(pathname);
            if(readable)
                return readable;
        }

        return _raw_asset_bundle->get(name);
    }

    sp<AssetBundle> getAssetBundle(const String& path) const {
        sp<AssetBundle> asset;
        const String s = strip(path);
        for(const Mounted& i : _mounts) {
            const sp<AssetBundle> ia = i.getBundle(s);
            if(ia)
                asset = asset ? sp<AssetBundle>::adopt(new AssetBundleWithFallback(asset, ia)) : ia;
        }
        const sp<AssetBundle> fallback = _raw_asset_bundle->getBundle(path);
        if(fallback)
            return asset ? sp<AssetBundle>::adopt(new AssetBundleWithFallback(asset, fallback)) : fallback;
        DCHECK(asset, "Asset \"%s\" doesn't exists", path.c_str());
        return asset;
    }

private:
    sp<AssetBundle> createAsset(const String& src) {
        if(Platform::isDirectory(src))
            return sp<AssetBundleDirectory>::make(src);
        else if(Platform::isFile(src))
            return sp<AssetBundleZipFile>::make(sp<FileReadable>::make(src, "rb"), src);
        const sp<AssetBundle> asset = _raw_asset_bundle->getBundle(src);
        DCHECK(asset, "Unknow asset src: %s", src.c_str());
        return asset;
    }

    String strip(const String& path) const {
        String s = path.lstrip('/').lstrip('.');

        while(s && !s.endsWith("/"))
            s = s + "/";
        return s;
    }

    class Mounted {
    public:
        Mounted(const String& prefix, const sp<AssetBundle>& asset)
            : _prefix(prefix), _asset(asset) {
        }

        sp<Asset> open(const String& path) const {
            if(_prefix.empty() || path.startsWith(_prefix))
                return _asset->get(path.substr(_prefix.length()));
            return nullptr;
        }

        sp<AssetBundle> getBundle(const String& path) const {
            if(path == _prefix)
                return _asset;
            if(path.startsWith(_prefix))
            {
                const String assetpath = path.substr(_prefix.length());
                const sp<AssetBundle> asset = _asset->getBundle(assetpath);
                if(asset)
                    return asset;

                const sp<Asset> fp = _asset->get(assetpath.rstrip('/'));
                if(fp)
                    return sp<AssetBundleZipFile>::make(fp->open(), Platform::getRealPath(assetpath.rstrip('/')));
            }
            return nullptr;
        }

    private:
        String _prefix;
        sp<AssetBundle> _asset;
    };

    List<Mounted> _mounts;
    sp<RawAssetBundle> _raw_asset_bundle;
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
    _asset_bundle = sp<ArkAssetBundle>::make(sp<RawAssetBundle>::make(manifest->assetDir(), manifest->appDir()), manifest->assets());

    loadPlugins(manifest);

    const sp<AssetBundle> asset = _asset_bundle->getAssetBundle(".");
    const sp<ApplicationResource> appResource = sp<ApplicationResource>::make(sp<XMLDirectory>::make(asset), asset);
    const sp<RenderEngine> renderEngine = createRenderEngine(manifest->renderer()._version, appResource);
    _application_context = createApplicationContext(manifest->content(), appResource, renderEngine);
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

sp<ApplicationContext> Ark::createApplicationContext(const document& manifest, const sp<ApplicationResource>& appResource, const sp<RenderEngine>& renderEngine)
{
    const Global<PluginManager> pluginManager;
    const sp<ApplicationContext> applicationContext = sp<ApplicationContext>::make(appResource, renderEngine);
    pluginManager->addPlugin(sp<ApplicationPlugin>::make(applicationContext));
    applicationContext->initResourceLoader(manifest);
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
        break;
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

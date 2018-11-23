/**
Copyright 2011 - 2018 Jason S Taylor

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
#include "core/impl/asset/asset_with_fallback.h"
#include "core/impl/asset/asset_with_prefix.h"
#include "core/impl/asset/directory_asset.h"
#include "core/impl/asset/zip_asset.h"
#include "core/impl/readable/file_readable.h"
#include "core/impl/dictionary/dictionary_by_attribute_name.h"
#include "core/impl/dictionary/xml_directory.h"
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

class RawAsset : public Asset {
public:
    RawAsset(const String& assetDir, const String& appDir)
        : _asset_dir(assetDir), _app_dir(appDir) {
    }

    virtual sp<Readable> get(const String& filepath) override {
        String dirname, filename;
        Strings::rcut(filepath, dirname, filename, '/');
        const sp<Asset> dir = getAsset(dirname);
        const sp<Readable> resource = dir ? dir->get(filename) : nullptr;
        LOGD("filepath(%s) dirname(%s) ==> dir<%p> asset<%p>", filepath.c_str(), dirname.c_str(), dir.get(), resource.get());
        return resource;
    }

    virtual sp<Asset> getAsset(const String& path) override {
        String s = (path.empty() || path == "/") ? "." : path;
        const String assetDir = Platform::pathJoin(_asset_dir, s);
        const sp<Asset> asset = Platform::getAsset(s, Platform::pathJoin(_app_dir, assetDir));
        if(asset)
            return asset;

        const sp<Readable> resource = get(path);
        if(resource)
            return sp<ZipAsset>::make(resource);

        String dirname;
        String filename;

        do {
            String name;
            Strings::rcut(s, dirname, name, '/');
            filename = filename.empty() ? name : name + "/" + filename;
            const sp<Readable> readable = dirname.empty() ? nullptr : get(dirname);
            if(readable) {
                const sp<ZipAsset> zip = sp<ZipAsset>::make(readable);
                const String entryName = filename + "/";
                return zip->hasEntry(entryName) ? sp<AssetWithPrefix>::make(zip, entryName) : nullptr;
            }
            s = dirname;
        } while(!dirname.empty());
        return nullptr;
    }

    String _asset_dir;
    String _app_dir;
};

}


class Ark::ArkAsset {
public:
    ArkAsset(const sp<RawAsset>& rawAsset, const document& manifest)
        : _raw_asset(rawAsset) {
        for(const document& i : manifest->children("asset")) {
            const String prefix = Documents::getAttribute(i, "prefix");
            const String& src = Documents::ensureAttribute(i, Constants::Attributes::SRC);
            _mounts.push_front(Mounted(strip(prefix), createAsset(src)));
        }
    }

    sp<Readable> getResource(const String& name) {
        String pathname = name;
        while(pathname.startsWith("/") || pathname.startsWith("."))
            pathname = pathname.substr(1);

        for(const Mounted& i : _mounts) {
            const sp<Readable> readable = i.open(pathname);
            if(readable)
                return readable;
        }

        return _raw_asset->get(name);
    }

    sp<Asset> getAsset(const String& path) const {
        sp<Asset> asset;
        const String s = strip(path);
        for(const Mounted& i : _mounts) {
            const sp<Asset> ia = i.getAsset(s);
            if(ia)
                asset = asset ? sp<Asset>::adopt(new AssetWithFallback(asset, ia)) : ia;
        }
        const sp<Asset> fallback = _raw_asset->getAsset(path);
        if(fallback)
            return asset ? sp<Asset>::adopt(new AssetWithFallback(asset, fallback)) : fallback;
        DCHECK(asset, "Asset \"%s\" doesn't exists", path.c_str());
        return asset;
    }

private:
    sp<Asset> createAsset(const String& src) {
        if(Platform::isDirectory(src))
            return sp<DirectoryAsset>::make(src);
        else if(Platform::isFile(src))
            return sp<ZipAsset>::make(sp<FileReadable>::make(src, "rb"));
        const sp<Asset> asset = _raw_asset->getAsset(src);
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
        Mounted(const String& prefix, const sp<Asset>& asset)
            : _prefix(prefix), _asset(asset) {
        }

        sp<Readable> open(const String& path) const {
            if(_prefix.empty() || path.startsWith(_prefix))
                return _asset->get(path.substr(_prefix.length()));
            return nullptr;
        }

        sp<Asset> getAsset(const String& path) const {
            if(path == _prefix)
                return _asset;
            if(path.startsWith(_prefix))
            {
                const String assetpath = path.substr(_prefix.length());
                const sp<Asset> asset = _asset->getAsset(assetpath);
                if(asset)
                    return asset;

                const sp<Readable> fp = _asset->get(assetpath.rstrip('/'));
                if(fp)
                    return sp<ZipAsset>::make(fp);
            }
            return nullptr;
        }

    private:
        String _prefix;
        sp<Asset> _asset;
    };

    List<Mounted> _mounts;
    sp<RawAsset> _raw_asset;
};

Ark::Ark(int32_t argc, const char** argv, const String& manfiestSrc)
    : _argc(argc), _argv(argv), _object_pool(sp<ObjectPool>::make())
{
    push();
    __ark_bootstrap__();

    String appFilename;
    String appDir;
    Strings::rcut(Platform::getExecutablePath(), appDir, appFilename, Platform::dirSeparator());

    const sp<Asset> appAsset = Platform::getAsset(".", appDir);
    DASSERT(appAsset);
    const sp<Readable> readable = manfiestSrc ? appAsset->get(manfiestSrc) : nullptr;
    DWARN(!manfiestSrc || readable, "Cannot load application manifest \"%s\"", manfiestSrc.c_str());
    _manifest = readable ? Documents::loadFromReadable(readable) : document::make("");
    const String& assetDir = Documents::getAttribute(_manifest, "asset-dir");
    _asset = sp<ArkAsset>::make(sp<RawAsset>::make(assetDir, appDir), _manifest);

    const sp<Asset> asset = _asset->getAsset(".");
    const sp<ApplicationResource> appResource = sp<ApplicationResource>::make(sp<XMLDirectory>::make(asset), asset);

    const sp<RenderEngine> renderEngine = createRenderEngine(Documents::getAttribute<GLVersion>(_manifest, "renderer", AUTO), appResource);
    _application_context = createApplicationContext(_manifest, appResource, renderEngine);

    loadPlugins(_manifest);
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
    DASSERT(_instance);
    return *_instance;
}

void Ark::push()
{
    _instance_stack.push_front(_instance);
    _instance = this;
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

const document& Ark::manifest() const
{
    return _manifest;
}

sp<Asset> Ark::getAsset(const String& path) const
{
    return _asset->getAsset(path);
}

sp<Readable> Ark::getResource(const String& path) const
{
    return _asset->getResource(path);
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

sp<RenderEngine> Ark::createRenderEngine(GLVersion version, const sp<ApplicationResource>& appResource)
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
        return sp<RenderEngine>::make(version, sp<opengl::RendererFactoryOpenGL>::make(appResource->glResourceManager()));
#endif
    case VULKAN_11:
#ifdef ARK_USE_VULKAN
        return sp<RenderEngine>::make(version, sp<vulkan::RendererFactoryVulkan>::make(appResource->glResourceManager()));
#endif
        break;
    }
    DFATAL("Unknown engine type: %d", version);
    return nullptr;
}

void Ark::loadPlugins(const document& manifest) const
{
    const Global<PluginManager> pluginManager;

    for(const document& i : manifest->children("plugin"))
        pluginManager->load(Documents::ensureAttribute(i, Constants::Attributes::NAME));
}

}

/**
Copyright 2011 - 2017 Jason S Taylor

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
#include "core/impl/asset/directory_asset.h"
#include "core/impl/asset/zip_asset.h"
#include "core/impl/readable/file_readable.h"
#include "core/impl/dictionary/dictionary_by_attribute_name.h"
#include "core/impl/dictionary/dictionary_with_prefix.h"
#include "core/impl/dictionary/xml_directory.h"
#include "core/types/global.h"

#include "renderer/base/render_engine.h"
#include "renderer/impl/render_view_factory/gles20_render_view_factory.h"

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
        const sp<Asset> dir = getRawAsset(dirname);
        const sp<Readable> resource = dir ? dir->get(filename) : nullptr;
        LOGD("filepath(%s) dirname(%s) ==> dir<%p> asset<%p>", filepath.c_str(), dirname.c_str(), dir.get(), resource.get());
        return resource;
    }

    sp<Asset> getRawAsset(const String& path) {
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
                return zip->hasEntry(entryName) ? sp<DictionaryWithPrefix<sp<Readable>>>::make(zip, entryName) : nullptr;
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
            const String& prefix = Documents::getAttribute(i, "prefix");
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
        const sp<Asset> fallback = _raw_asset->getRawAsset(path);
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
        else {
            const sp<Asset> asset = _raw_asset->getRawAsset(src);
            DCHECK(asset, "Unknow asset src: %s", src.c_str());
            return asset;
        }
        return nullptr;
    }

    String strip(const String& path) const {
        String s = path;
        while(s.startsWith("/") || s.startsWith("."))
            s = s.substr(1);

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
            if(_prefix.empty() || path == _prefix)
                return _asset;
            if(path.startsWith(_prefix))
                return sp<DictionaryWithPrefix<sp<Readable>>>::make(_asset, path.substr(_prefix.length()));
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
    : _argc(argc), _argv(argv)
{
    push();
    __ark_bootstrap__();

    String appFilename;
    String appDir;
    Strings::rcut(Platform::getExecutablePath(), appDir, appFilename, Platform::dirSeparator());

    const sp<Asset> appAsset = Platform::getAsset(".", appDir);
    NOT_NULL(appAsset);
    const sp<Readable> readable = manfiestSrc ? appAsset->get(manfiestSrc) : nullptr;
    DWARN(!manfiestSrc || readable, "Cannot load application manifest \"%s\"", manfiestSrc.c_str());
    _manifest = readable ? Documents::loadFromReadable(readable) : document::make("");
    const String& assetDir = Documents::getAttribute(_manifest, "asset-dir");
    _asset = sp<ArkAsset>::make(sp<RawAsset>::make(assetDir, appDir), _manifest);
    _application_context = createApplicationContext(_manifest);
    put<RenderEngine>(createRenderEngine(static_cast<GLVersion>(Documents::getAttribute<int32_t>(_manifest, "gl-version", 0))));

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
    NOT_NULL(_instance);
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

sp<ApplicationContext> Ark::createApplicationContext(const document& manifest)
{
    const Global<PluginManager> pluginManager;
    const sp<Asset> asset = _asset->getAsset(".");
    const sp<ApplicationResource> applicationResource = sp<ApplicationResource>::make(sp<Dictionary<document>>::adopt(new XMLDirectory(asset)), asset);
    const sp<ApplicationContext> applicationContext = sp<ApplicationContext>::make(applicationResource);
    pluginManager->addPlugin(sp<ApplicationPlugin>::make(applicationContext));
    applicationContext->initResourceLoader(manifest);
    return applicationContext;
}

sp<RenderEngine> Ark::createRenderEngine(GLVersion version)
{
    const sp<RenderViewFactory> renderViewFactory = sp<GLES20RenderViewFactory>::make(_application_context->applicationResource()->glResourceManager());

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
        return sp<RenderEngine>::make(version, renderViewFactory);
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

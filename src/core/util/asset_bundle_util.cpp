#include "core/util/asset_bundle_util.h"

#include "core/impl/asset_bundle/asset_bundle_directory.h"
#include "core/impl/asset_bundle/asset_bundle_with_prefix.h"
#include "core/impl/asset_bundle/asset_bundle_zip_file.h"
#include "core/impl/readable/file_readable.h"
#include "core/inf/asset.h"
#include "core/util/strings.h"

#include "platform/platform.h"

namespace ark {

namespace {

class BuiltInAssetBundle : public AssetBundle {
public:
    BuiltInAssetBundle(String assetDir, String appDir)
        : _asset_dir(std::move(assetDir)), _app_dir(std::move(appDir)) {
    }

    virtual sp<Asset> getAsset(const String& filepath) override {
        const auto [dirname, filename] = filepath.rcut('/');
        const sp<AssetBundle> dir = getBundle(dirname ? dirname.value() : "");
        const sp<Asset> asset = dir ? dir->getAsset(filename) : nullptr;
        LOGD("filepath(%s) dirname(%s) ==> dir<%p> asset<%p>", filepath.c_str(), dirname ? dirname.value().c_str() : "", dir.get(), asset.get());
        return asset;
    }

    virtual sp<AssetBundle> getBundle(const String& path) override {
        String s = (path.empty() || path == "/") ? "." : path;
        const String assetDir = Platform::pathJoin(_asset_dir, s);
        const sp<AssetBundle> assetBundle = Platform::getAssetBundle(s, Platform::pathJoin(_app_dir, assetDir));
        if(assetBundle)
            return assetBundle;

        const sp<Asset> asset = getAsset(path);
        if(asset) {
            const sp<Readable> readable = asset->open();
            if(readable)
                return sp<AssetBundleZipFile>::make(asset->open(), path);
        }

        String dirname;
        String filename;

        do {
            String name;
            Strings::rcut(s, dirname, name, '/');
            filename = filename.empty() ? name : name + "/" + filename;
            const sp<Asset> asset = dirname.empty() ? nullptr : getAsset(dirname);
            sp<Readable> readable = asset ? asset->open() : nullptr;
            if(readable) {
                const sp<AssetBundleZipFile> zip = sp<AssetBundleZipFile>::make(std::move(readable), dirname);
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

sp<Asset> AssetBundleUtil::get(const sp<AssetBundle>& self, const String& name)
{
    return self->getAsset(name);
}

sp<AssetBundle> AssetBundleUtil::getBundle(const sp<AssetBundle>& self, const String& path)
{
    return self->getBundle(path);
}

sp<String> AssetBundleUtil::getString(const sp<AssetBundle>& self, const String& filepath)
{
    const sp<Asset> asset = self->getAsset(filepath);
    const sp<Readable> readable = asset ? asset->open() : nullptr;
    return readable ? sp<String>::make(Strings::loadFromReadable(readable)) : nullptr;
}

String AssetBundleUtil::getRealPath(const sp<AssetBundle>& self, const String& filepath)
{
    const sp<Asset> asset = self->getAsset(filepath);
    return asset ? asset->location() : filepath;
}

sp<AssetBundle> AssetBundleUtil::createBuiltInAssetBundle(const String& assetDir, const String& appDir)
{
    return sp<BuiltInAssetBundle>::make(assetDir, appDir);
}

sp<AssetBundle> AssetBundleUtil::createFileAssetBundle(const String& filepath)
{
    if(Platform::isDirectory(filepath))
        return sp<AssetBundleDirectory>::make(filepath);
    else if(Platform::isFile(filepath))
        return sp<AssetBundleZipFile>::make(sp<FileReadable>::make(filepath, "rb"), filepath);
    return nullptr;
}

AssetBundleUtil::FILE_DICTIONARY::FILE_DICTIONARY(BeanFactory& /*factory*/, const String& src)
    : _src(src)
{
}

sp<AssetBundle> AssetBundleUtil::FILE_DICTIONARY::build(const Scope& /*args*/)
{
    return AssetBundleUtil::createFileAssetBundle(_src);
}

AssetBundleUtil::EXTERNAL_DICTIONARY::EXTERNAL_DICTIONARY(BeanFactory& /*factory*/, const String& src)
    : _src(Platform::getExternalStoragePath(src))
{
}

sp<AssetBundle> AssetBundleUtil::EXTERNAL_DICTIONARY::build(const Scope& /*args*/)
{
    return AssetBundleUtil::createFileAssetBundle(_src);
}

}

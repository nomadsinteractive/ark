#include "core/util/asset_bundle_type.h"

#include "core/impl/asset_bundle/asset_bundle_directory.h"
#include "core/impl/asset_bundle/asset_bundle_with_prefix.h"
#include "core/impl/asset_bundle/asset_bundle_zip_file.h"
#include "core/impl/readable/file_readable.h"
#include "core/inf/asset.h"
#include "core/types/optional.h"
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

        String filename;
        do {
            const auto [dirnameOpt, name] = s.rcut('/');
            filename = filename.empty() ? name : name + "/" + filename;
            const sp<Asset> asset = dirnameOpt ? nullptr : getAsset(dirnameOpt ? dirnameOpt.value() : "");
            sp<Readable> readable = asset ? asset->open() : nullptr;
            if(readable) {
                sp<AssetBundleZipFile> zip = sp<AssetBundleZipFile>::make(std::move(readable), dirnameOpt.value());
                String entryName = filename + "/";
                return zip->hasEntry(entryName) ? sp<AssetBundleWithPrefix>::make(std::move(zip), std::move(entryName)) : nullptr;
            }
            s = dirnameOpt ? dirnameOpt.value() : "";
        } while(!s.empty());
        return nullptr;
    }

    virtual std::vector<sp<Asset>> listAssets(const String& regex) override {
        DFATAL(false, "Unimplemented");
        return {};
    }

    String _asset_dir;
    String _app_dir;

};

}

std::vector<sp<Asset>> AssetBundleType::listAssets(const sp<AssetBundle>& self, const String& regex)
{
    return self->listAssets(regex);
}

sp<Asset> AssetBundleType::get(const sp<AssetBundle>& self, const String& name)
{
    return self->getAsset(name);
}

sp<AssetBundle> AssetBundleType::getBundle(const sp<AssetBundle>& self, const String& path)
{
    return self->getBundle(path);
}

sp<String> AssetBundleType::getString(const sp<AssetBundle>& self, const String& filepath)
{
    const sp<Asset> asset = self->getAsset(filepath);
    const sp<Readable> readable = asset ? asset->open() : nullptr;
    return readable ? sp<String>::make(Strings::loadFromReadable(readable)) : nullptr;
}

String AssetBundleType::getRealPath(const sp<AssetBundle>& self, const String& filepath)
{
    const sp<Asset> asset = self->getAsset(filepath);
    return asset ? asset->location() : filepath;
}

sp<AssetBundle> AssetBundleType::createBuiltInAssetBundle(const String& assetDir, const String& appDir)
{
    return sp<BuiltInAssetBundle>::make(assetDir, appDir);
}

sp<AssetBundle> AssetBundleType::createFileAssetBundle(const String& filepath)
{
    if(Platform::isDirectory(filepath))
        return sp<AssetBundleDirectory>::make(filepath);
    else if(Platform::isFile(filepath))
        return sp<AssetBundleZipFile>::make(sp<FileReadable>::make(filepath, "rb"), filepath);
    return nullptr;
}

AssetBundleType::FILE_DICTIONARY::FILE_DICTIONARY(BeanFactory& /*factory*/, const String& src)
    : _src(src)
{
}

sp<AssetBundle> AssetBundleType::FILE_DICTIONARY::build(const Scope& /*args*/)
{
    return AssetBundleType::createFileAssetBundle(_src);
}

AssetBundleType::EXTERNAL_DICTIONARY::EXTERNAL_DICTIONARY(BeanFactory& /*factory*/, const String& src)
    : _src(Platform::getExternalStoragePath(src))
{
}

sp<AssetBundle> AssetBundleType::EXTERNAL_DICTIONARY::build(const Scope& /*args*/)
{
    return AssetBundleType::createFileAssetBundle(_src);
}

}

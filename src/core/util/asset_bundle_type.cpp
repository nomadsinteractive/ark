#include "core/util/asset_bundle_type.h"

#include "core/impl/asset_bundle/asset_bundle_directory.h"
#include "core/impl/asset_bundle/asset_bundle_with_prefix.h"
#include "core/impl/asset_bundle/asset_bundle_zip_file.h"
#include "core/impl/readable/file_readable.h"
#include "core/inf/array.h"
#include "core/inf/asset.h"
#include "core/types/optional.h"
#include "core/util/log.h"
#include "core/util/strings.h"

#include "platform/platform.h"

namespace ark {

namespace {

class AssetBundleDefault final : public AssetBundle {
public:

    sp<Asset> getAsset(const String& filepath) override
    {
        const auto [dirname, filename] = filepath.rcut('/');
        const sp<AssetBundle> dir = getBundle(dirname ? dirname.value() : "");
        const sp<Asset> asset = dir ? dir->getAsset(filename) : nullptr;
        LOGD("filepath(%s) dirname(%s) ==> dir<%p> asset<%p>", filepath.c_str(), dirname ? dirname.value().c_str() : "", dir.get(), asset.get());
        return asset;
    }

    sp<AssetBundle> getBundle(const String& path) override
    {
        String assetDir = (path.empty() || path == "/") ? "." : path;

        if(sp<AssetBundle> assetBundle = Platform::getAssetBundle(assetDir))
            return assetBundle;

        if(const sp<Asset> asset = getAsset(path))
            if(sp<Readable> readable = asset->open())
                return sp<AssetBundle>::make<AssetBundleZipFile>(std::move(readable), path);

        String filename;
        do {
            const auto [dirnameOpt, name] = assetDir.rcut('/');
            String dirname = dirnameOpt ? dirnameOpt.value() : "";
            filename = filename.empty() ? name : name + "/" + filename;
            const sp<Asset> asset = getAsset(dirname);
            if(sp<Readable> readable = asset ? asset->open() : nullptr)
            {
                sp<AssetBundleZipFile> zip = sp<AssetBundleZipFile>::make(std::move(readable), dirname);
                String entryName = filename + "/";
                return zip->hasEntry(entryName) ? sp<AssetBundle>::make<AssetBundleWithPrefix>(std::move(zip), std::move(entryName)) : nullptr;
            }
            assetDir = std::move(dirname);
        } while(!assetDir.empty());

        return nullptr;
    }

    Vector<String> listAssets(StringView dirname) override {
        DFATAL("Unimplemented");
        return {};
    }
};

class ByteArrayString final : public ByteArray {
public:
    ByteArrayString(String bytes)
        : _bytes(std::move(bytes)) {
    }

    size_t length() override
    {
        return _bytes.length();
    }

    uint8_t* buf() override
    {
        return const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(_bytes.c_str()));
    }

private:
    String _bytes;
};

}

Vector<String> AssetBundleType::listAssets(const sp<AssetBundle>& self, const StringView dirname)
{
    return self->listAssets(dirname);
}

sp<Asset> AssetBundleType::getAsset(const sp<AssetBundle>& self, const String& name)
{
    return self->getAsset(name);
}

sp<AssetBundle> AssetBundleType::getBundle(const sp<AssetBundle>& self, const String& path)
{
    return self->getBundle(path);
}

Optional<String> AssetBundleType::getString(const sp<AssetBundle>& self, const String& filepath)
{
    const sp<Asset> asset = self->getAsset(filepath);
    const sp<Readable> readable = asset ? asset->open() : nullptr;
    return readable ? Optional<String>(Strings::loadFromReadable(readable)) : Optional<String>();
}

bytearray AssetBundleType::getByteArray(const sp<AssetBundle>& self, const String& filepath)
{
    if(Optional<String> bytes = getString(self, filepath))
        return sp<ByteArray>::make<ByteArrayString>(std::move(bytes.value()));
    return nullptr;
}

String AssetBundleType::getRealPath(const sp<AssetBundle>& self, const String& filepath)
{
    const sp<Asset> asset = self->getAsset(filepath);
    return asset ? asset->location() : filepath;
}

sp<AssetBundle> AssetBundleType::createBuiltInAssetBundle()
{
    return sp<AssetBundle>::make<AssetBundleDefault>();
}

sp<AssetBundle> AssetBundleType::createAssetBundle(const String& filepath)
{
    if(Platform::isDirectory(filepath))
        return sp<AssetBundle>::make<AssetBundleDirectory>(filepath);
    if(Platform::isFile(filepath))
        return sp<AssetBundle>::make<AssetBundleZipFile>(sp<Readable>::make<FileReadable>(filepath, "rb"), filepath);
    return nullptr;
}

}

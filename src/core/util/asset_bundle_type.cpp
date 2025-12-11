#include "core/util/asset_bundle_type.h"

#include <filesystem>

#include "core/impl/asset_bundle/asset_bundle_compound.h"
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

class AssetBundleComposite final : public AssetBundle {
public:
    AssetBundleComposite(Vector<sp<AssetBundle>> assetBundles)
        : _asset_bundles(std::move(assetBundles)) {
    }

    sp<Asset> getAsset(const String& path) override
    {
        for(const sp<AssetBundle>& i : _asset_bundles)
            if(sp<Asset> asset = i->getAsset(path))
                return asset;
        return nullptr;
    }

    sp<AssetBundle> getBundle(const String& path) override
    {
        sp<AssetBundle> assetBundle = nullptr;
        for(const sp<AssetBundle>& i : _asset_bundles)
            if(sp<AssetBundle> b = i->getBundle(path))
                assetBundle = assetBundle ? sp<AssetBundle>::make<AssetBundleCompound>(std::move(assetBundle), std::move(b)) : std::move(b);

        return assetBundle;
    }

    Vector<String> listAssets(const StringView dirname) override
    {
        Set<String> assetNameSet;
        for(const sp<AssetBundle>& i : _asset_bundles)
            for(String& j : i->listAssets(dirname))
                assetNameSet.insert(std::move(j));

        Vector<String> assetNames;
        for(const String& i : assetNameSet)
            assetNames.push_back(std::move(i));
        return assetNames;
    }

private:
    Vector<sp<AssetBundle>> _asset_bundles;
};

class AssetBundleDefault final : public AssetBundle {
public:
    AssetBundleDefault(sp<AssetBundle> root)
        : _root(std::move(root)) {
    }

    sp<Asset> getAsset(const String& filepath) override
    {
        if(sp<Asset> asset = _root->getAsset(filepath))
        {
            LOGD("asset(%s) ==> location<%s>", filepath.c_str(), asset->location().c_str());
            return asset;
        }
        return nullptr;
    }

    sp<AssetBundle> getBundle(const String& path) override
    {
        String assetDir = (path.empty() || path == "/") ? "." : path;

        if(const sp<Asset> asset = getAsset(assetDir))
            if(sp<Readable> readable = asset->open())
                return sp<AssetBundle>::make<AssetBundleZipFile>(std::move(readable), path, asset->size());

        if(sp<AssetBundle> assetBundle = _root->getBundle(assetDir))
            return assetBundle;

        String filename;
        do {
            const auto [dirnameOpt, name] = assetDir.rcut('/');
            String dirname = dirnameOpt ? dirnameOpt.value() : "";
            filename = filename.empty() ? name : name + "/" + filename;
            const sp<Asset> asset = getAsset(dirname);
            if(sp<Readable> readable = asset ? asset->open() : nullptr)
            {
                sp<AssetBundleZipFile> zip = sp<AssetBundleZipFile>::make(std::move(readable), dirname, asset->size());
                String entryName = filename + "/";
                return zip->hasEntry(entryName) ? sp<AssetBundle>::make<AssetBundleWithPrefix>(std::move(zip), std::move(entryName)) : nullptr;
            }
            assetDir = std::move(dirname);
        } while(!assetDir.empty());

        return nullptr;
    }

    Vector<String> listAssets(const StringView dirname) override {
        return _root->listAssets(dirname);
    }

private:
    sp<AssetBundle> _root;
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
    const sp<AssetBundle> currentDir = Platform::getAssetBundle(".");
    Vector<sp<AssetBundle>> assetBundles = {sp<AssetBundle>::make<AssetBundleDefault>(Platform::getAssetBundle("."))};
    for(const String& i : currentDir->listAssets("."))
        if(i.endsWith(".ark"))
            assetBundles.push_back(sp<AssetBundle>::make<AssetBundleDefault>(sp<AssetBundle>::make<AssetBundleZipFile>(currentDir->getAsset(i))));
    return sp<AssetBundle>::make<AssetBundleComposite>(std::move(assetBundles));
}

sp<AssetBundle> AssetBundleType::createAssetBundle(const String& filepath)
{
    if(Platform::isDirectory(filepath))
        return sp<AssetBundle>::make<AssetBundleDirectory>(filepath);
    if(Platform::isFile(filepath))
        return sp<AssetBundle>::make<AssetBundleZipFile>(sp<Readable>::make<FileReadable>(filepath, "rb"), filepath, std::filesystem::file_size(filepath.c_str()));
    return nullptr;
}

}

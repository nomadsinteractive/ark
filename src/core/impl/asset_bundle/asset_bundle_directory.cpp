#include "core/impl/asset_bundle/asset_bundle_directory.h"

#include <filesystem>

#include "core/impl/asset/asset_file.h"
#include "core/types/shared_ptr.h"
#include "core/util/strings.h"

#include "platform/platform.h"

namespace ark {

AssetBundleDirectory::AssetBundleDirectory(String directory)
    : _directory(std::move(directory))
{
}

sp<Asset> AssetBundleDirectory::getAsset(const String& name)
{
    const String filepath = Platform::isAbsolutePath(name) ? name : Platform::pathJoin(_directory, name);
    if(Platform::isFile(filepath))
        return sp<Asset>::make<AssetFile>(filepath);
    return nullptr;
}

sp<AssetBundle> AssetBundleDirectory::getBundle(const String& path)
{
    const String dirname = Platform::pathJoin(_directory, path);
    if(Platform::isDirectory(dirname))
        return sp<AssetBundleDirectory>::make(dirname);
    return nullptr;
}

Vector<String> AssetBundleDirectory::listAssets()
{
    Vector<String> assets;
    if(Platform::isDirectory(_directory))
    {
        for(const std::filesystem::path& i : std::filesystem::directory_iterator(_directory.c_str()))
            assets.emplace_back(i.filename().string());
    }
    return assets;
}

}

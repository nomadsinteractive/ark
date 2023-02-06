#include "core/impl/asset_bundle/asset_bundle_directory.h"

#include <filesystem>

#include "core/impl/asset/asset_file.h"
#include "core/types/shared_ptr.h"

#include "platform/platform.h"

namespace ark {

AssetBundleDirectory::AssetBundleDirectory(String directory)
    : _directory(std::move(directory))
{
}

sp<Asset> AssetBundleDirectory::getAsset(const String& name)
{
    const String filepath = Platform::isAbsolutePath(name) ? name : Strings::sprintf("%s/%s", _directory.c_str(), name.c_str());
    if(Platform::isFile(filepath))
        return sp<AssetFile>::make(filepath);
    return nullptr;
}

sp<AssetBundle> AssetBundleDirectory::getBundle(const String& path)
{
    const String dirname = Platform::pathJoin(_directory, path);
    if(Platform::isDirectory(dirname))
        return sp<AssetBundleDirectory>::make(dirname);
    return nullptr;
}

std::vector<sp<Asset>> AssetBundleDirectory::listAssets(const String& regex)
{
    std::vector<sp<Asset>> assets;
    if(Platform::isDirectory(_directory))
    {
        const std::regex namePattern(regex.c_str());
        for(const std::filesystem::path& i : std::filesystem::directory_iterator(_directory.c_str()))
        {
            if(regex.empty() || std::regex_match(i.filename().string(), namePattern))
                assets.push_back(sp<AssetFile>::make(i.string()));
        }
    }
    return assets;
}

}

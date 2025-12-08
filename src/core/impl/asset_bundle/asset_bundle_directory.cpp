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
    if(const String filepath = Platform::isAbsolutePath(name) ? name : Platform::pathJoin(_directory, name); Platform::isFile(filepath))
        return sp<Asset>::make<AssetFile>(filepath);
    return nullptr;
}

sp<AssetBundle> AssetBundleDirectory::getBundle(const String& path)
{
    if(const String dirname = Platform::pathJoin(_directory, path); Platform::isDirectory(dirname))
        return sp<AssetBundle>::make<AssetBundleDirectory>(dirname);
    return nullptr;
}

Vector<String> AssetBundleDirectory::listAssets(const StringView dirname)
{
    Vector<String> assets;
    const String dname = Platform::pathJoin(_directory, dirname);
    if(Platform::isDirectory(dname))
    {
        for(const std::filesystem::path& i : std::filesystem::directory_iterator(dname.c_str()))
            assets.emplace_back(i.relative_path().filename().string());
    }
    return assets;
}

}

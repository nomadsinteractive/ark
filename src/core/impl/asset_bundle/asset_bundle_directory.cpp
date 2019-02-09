#include "core/impl/asset_bundle/asset_bundle_directory.h"

#include "core/impl/asset/asset_file.h"
#include "core/types/shared_ptr.h"

#include "platform/platform.h"

namespace ark {

AssetBundleDirectory::AssetBundleDirectory(const String& directory)
    : _directory(directory)
{
}

sp<Asset> AssetBundleDirectory::get(const String& name)
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

}

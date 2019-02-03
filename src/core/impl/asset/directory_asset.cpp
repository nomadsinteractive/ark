#include "core/impl/asset/directory_asset.h"

#include "core/impl/readable/file_readable.h"
#include "core/types/shared_ptr.h"

#include "platform/platform.h"

namespace ark {

DirectoryAsset::DirectoryAsset(const String& directory)
    : _directory(directory)
{
}

sp<Readable> DirectoryAsset::get(const String& name)
{
    const String filepath = Platform::isAbsolutePath(name) ? name : Strings::sprintf("%s/%s", _directory.c_str(), name.c_str());
    if(Platform::isFile(filepath))
    {
        FILE* fp = fopen(filepath.c_str(), "rb");
        DCHECK(fp, "Open file \"%s\" failed", filepath.c_str());
        return sp<FileReadable>::make(fp);
    }
    return nullptr;
}

sp<Asset> DirectoryAsset::getAsset(const String& path)
{
    const String dirname = Platform::pathJoin(_directory, path);
    if(Platform::isDirectory(dirname))
        return sp<DirectoryAsset>::make(dirname);
    return nullptr;
}

String DirectoryAsset::getRealPath(const String& path)
{
    return Platform::isAbsolutePath(path) ? path : Platform::getRealPath(Strings::sprintf("%s/%s", _directory.c_str(), path.c_str()));
}

}

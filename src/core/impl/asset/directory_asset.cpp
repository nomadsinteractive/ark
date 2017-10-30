#include "core/impl/asset/directory_asset.h"

#include <stdio.h>

#include "core/impl/readable/file_readable.h"

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

}

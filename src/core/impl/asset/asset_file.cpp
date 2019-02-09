#include "core/impl/asset/asset_file.h"

#include "core/impl/readable/file_readable.h"

#include "platform/platform.h"

namespace ark {

AssetFile::AssetFile(const String& filepath)
    : _filepath(filepath)
{
}

sp<Readable> AssetFile::open()
{
    return sp<FileReadable>::make(_filepath, "rb");
}

String AssetFile::location()
{
    return Platform::getRealPath(_filepath);
}

}

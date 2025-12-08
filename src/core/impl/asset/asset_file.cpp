#include "core/impl/asset/asset_file.h"

#include <filesystem>

#include "core/impl/readable/file_readable.h"

#include "platform/platform.h"

namespace ark {

AssetFile::AssetFile(String filepath)
    : _filepath(std::move(filepath))
{
}

sp<Readable> AssetFile::open()
{
    return sp<Readable>::make<FileReadable>(_filepath, "rb");
}

String AssetFile::location()
{
    return Platform::getRealPath(_filepath);
}

size_t AssetFile::size()
{
    return std::filesystem::file_size(_filepath.c_str());
}

}

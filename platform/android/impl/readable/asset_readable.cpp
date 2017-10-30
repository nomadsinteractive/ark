#include "platform/android/impl/readable/asset_readable.h"

#include <stdio.h>

namespace ark {
namespace platform {
namespace android {

AssetReadable::AssetReadable(AAsset* asset)
    : _asset(asset)
{
}

AssetReadable::~AssetReadable()
{
    AAsset_close(_asset);
}

uint32_t AssetReadable::read(void* buffer, uint32_t bufferSize)
{
    return AAsset_read(_asset, buffer, bufferSize);
}

int32_t AssetReadable::seek(int32_t position, int32_t whence)
{
    return AAsset_seek(_asset, position, whence);
}

int32_t AssetReadable::remaining()
{
	return AAsset_getRemainingLength(_asset);
}

}
}
}

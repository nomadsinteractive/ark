#include "assimp/impl/io/ark_io_system.h"

#include "core/ark.h"

#include "assimp/impl/io/ark_io_stream.h"
#include "core/inf/asset.h"

using namespace Assimp;

namespace ark {
namespace plugin {
namespace assimp {

bool ArkIOSystem::Exists(const char* pFile) const
{
    return static_cast<bool>(Ark::instance().getAsset(pFile));
}

char ArkIOSystem::getOsSeparator() const
{
    return '/';
}

IOStream* ArkIOSystem::Open(const char* pFile, const char* pMode)
{
    const String mode = pMode;
    if(mode == "r" || mode == "rb")
    {
        const sp<Asset> asset = Ark::instance().getAsset(pFile);
        if(asset)
            return new ArkIOStream(asset->open(), asset->size());
    }
    return nullptr;
}

void ArkIOSystem::Close(IOStream* pFile)
{
    delete pFile;
}

}
}
}

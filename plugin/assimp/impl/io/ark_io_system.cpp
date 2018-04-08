#include "assimp/impl/io/ark_io_system.h"

#include "core/ark.h"

#include "assimp/impl/io/ark_io_stream.h"

using namespace Assimp;

namespace ark {
namespace plugin {
namespace assimp {

bool ArkIOSystem::Exists(const char* pFile) const
{
    const sp<Readable> fp = Ark::instance().getResource(pFile);
    return static_cast<bool>(fp);
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
        const sp<Readable> fp = Ark::instance().getResource(pFile);
        return new ArkIOStream(fp);
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

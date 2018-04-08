#include "assimp/impl/io/ark_io_stream.h"

#include "core/inf/readable.h"

using namespace Assimp;

namespace ark {
namespace plugin {
namespace assimp {

ArkIOStream::ArkIOStream(const sp<Readable>& readable)
    : _readable(readable), _file_size(readable->remaining())
{
}

size_t ArkIOStream::Read(void* pvBuffer, size_t pSize, size_t pCount)
{
    return _readable->read(pvBuffer, pSize * pCount);
}

size_t ArkIOStream::Write(const void* pvBuffer, size_t pSize, size_t pCount)
{
    return 0;
}

aiReturn ArkIOStream::Seek(size_t pOffset, aiOrigin pOrigin)
{
    _readable->seek(pOffset, static_cast<int32_t>(pOrigin));
    return aiReturn_SUCCESS;
}

size_t ArkIOStream::Tell() const
{
    return _file_size - _readable->remaining();
}

size_t ArkIOStream::FileSize() const
{
    return _file_size;
}

void ArkIOStream::Flush()
{
}

}
}
}

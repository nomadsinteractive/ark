#ifndef ARK_PLUGIN_ASSIMP_IMPL_IO_ARK_IO_STREAM_H_
#define ARK_PLUGIN_ASSIMP_IMPL_IO_ARK_IO_STREAM_H_

#include <assimp/IOStream.hpp>

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

namespace ark {
namespace plugin {
namespace assimp {

class ArkIOStream : public Assimp::IOStream {
public:
    ArkIOStream(const sp<Readable>& readable);

    virtual size_t Read(void* pvBuffer,
        size_t pSize,
        size_t pCount);

    virtual size_t Write(const void* pvBuffer,
        size_t pSize,
        size_t pCount);

    virtual aiReturn Seek(size_t pOffset,
        aiOrigin pOrigin);

    virtual size_t Tell() const;

    virtual size_t FileSize() const;

    virtual void Flush();

private:
    sp<Readable> _readable;
    size_t _file_size;
};

}
}
}

#endif

#pragma once

#include <assimp/IOStream.hpp>

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

namespace ark::plugin::assimp {

class ArkIOStream : public Assimp::IOStream {
public:
    ArkIOStream(sp<Readable> readable, size_t fileSize);

    size_t Read(void* pvBuffer, size_t pSize, size_t pCount) override;
    size_t Write(const void* pvBuffer, size_t pSize, size_t pCount) override;
    aiReturn Seek(size_t pOffset, aiOrigin pOrigin) override;
    size_t Tell() const override;
    size_t FileSize() const override;
    void Flush() override;

private:
    sp<Readable> _readable;
    size_t _file_size;
};

}

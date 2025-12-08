#pragma once

#include "core/forwarding.h"
#include "core/inf/readable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API BytearrayReadable final : public Readable {
public:
    BytearrayReadable(bytearray array);

    uint32_t read(void* buffer, uint32_t size) override;
    int32_t seek(int32_t position, int32_t whence) override;
    int32_t remaining() override;
    uint32_t position() override;

private:
    bytearray _bytearray;
    int32_t _position;
};

}

#pragma once

#include <stdint.h>

#include "core/base/api.h"

namespace ark {

class ARK_API Readable {
public:
    virtual ~Readable() = default;

    virtual uint32_t read(void* buffer, uint32_t size) = 0;
    virtual int32_t seek(int32_t position, int32_t whence) = 0;
    virtual uint32_t position() = 0;
};

}

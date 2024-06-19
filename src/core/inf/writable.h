#pragma once

#include <stdint.h>

#include "core/base/api.h"

namespace ark {

//[[script::bindings::auto]]
class ARK_API Writable {
public:
    virtual ~Writable() = default;

    virtual uint32_t write(const void* buffer, uint32_t size, uint32_t offset) = 0;

    template<typename T> uint32_t writeObject(const T& obj, uint32_t size = sizeof(T), uint32_t offset = 0) {
        return write(&obj, size, offset);
    }
};

}

#ifndef ARK_CORE_INF_WRITABLE_INF_
#define ARK_CORE_INF_WRITABLE_INF_

#include <stdint.h>

#include "core/base/api.h"

namespace ark {

class ARK_API Writable {
public:
    virtual ~Writable() = default;

    virtual uint32_t write(const void* buffer, uint32_t size, uint32_t offset) = 0;

};

}

#endif

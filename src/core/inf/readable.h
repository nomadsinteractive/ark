#ifndef ARK_CORE_INF_READABLE_INF_
#define ARK_CORE_INF_READABLE_INF_

#include <stdint.h>

#include "core/base/api.h"

namespace ark {

//[[script::bindings::auto]]
class ARK_API Readable {
public:
    virtual ~Readable() = default;

    virtual uint32_t read(void* buffer, uint32_t size) = 0;
    virtual int32_t seek(int32_t position, int32_t whence) = 0;
    virtual int32_t remaining() = 0;
};

}

#endif

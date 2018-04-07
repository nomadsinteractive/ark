#ifndef ARK_CORE_IMPL_READABLE_BYTEARRAY_READABLE_H_
#define ARK_CORE_IMPL_READABLE_BYTEARRAY_READABLE_H_

#include "core/forwarding.h"
#include "core/inf/readable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API BytearrayReadable : public Readable {
public:
    BytearrayReadable(const bytearray& array);

    virtual uint32_t read(void* buffer, uint32_t size) override;
    virtual int32_t seek(int32_t position, int32_t whence) override;
    virtual int32_t remaining() override;

private:
    bytearray _bytearray;
    int32_t _position;
};

}

#endif

#ifndef ARK_CORE_IMPL_WRITABLE_WRITABLE_WITH_OFFSET_H_
#define ARK_CORE_IMPL_WRITABLE_WRITABLE_WITH_OFFSET_H_

#include "core/inf/writable.h"

namespace ark {

class WritableWithOffset : public Writable {
public:
    WritableWithOffset(Writable& writable, size_t offset);

    virtual uint32_t write(const void* buffer, uint32_t size, uint32_t offset) override;

    void setOffset(size_t offset);

private:
    Writable& _writable;
    size_t _offset;
};

}

#endif

#ifndef ARK_CORE_IMPL_WRITABLE_WRITABLE_MEMORY_H_
#define ARK_CORE_IMPL_WRITABLE_WRITABLE_MEMORY_H_

#include "core/inf/writable.h"

namespace ark {

class WritableMemory : public Writable {
public:
    WritableMemory(void* ptr);

    virtual uint32_t write(const void* buffer, uint32_t size, uint32_t offset) override;
    virtual void flush() override;

private:
    uint8_t* _ptr;
};

}

#endif

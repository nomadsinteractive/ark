#pragma once

#include "core/inf/writable.h"

namespace ark {

class WritableMemory final : public Writable {
public:
    WritableMemory(void* ptr);

    uint32_t write(const void* buffer, uint32_t size, uint32_t offset) override;

private:
    uint8_t* _ptr;
};

}

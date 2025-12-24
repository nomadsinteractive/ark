#pragma once

#include "core/inf/writable.h"

namespace ark {

class WritableWithOffset : public Writable {
public:
    WritableWithOffset(Writable& writable, uint32_t offset);

    uint32_t write(const void* buffer, uint32_t size, uint32_t offset) override;

    void setOffset(size_t offset);

private:
    Writable& _writable;
    uint32_t _offset;
};

}

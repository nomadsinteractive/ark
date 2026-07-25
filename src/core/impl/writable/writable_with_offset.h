#pragma once

#include "core/inf/writable.h"

namespace ark {

class ARK_API WritableWithOffset final : public Writable {
public:
    WritableWithOffset(Writable& writable, int32_t offset);

    uint32_t write(const void* buffer, uint32_t size, uint32_t offset) override;

    void setOffset(int32_t offset);

private:
    Writable& _writable;
    int32_t _offset;
};

}

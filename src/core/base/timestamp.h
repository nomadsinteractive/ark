#pragma once

#include "core/base/api.h"

namespace ark {

class ARK_API Timestamp {
public:
    Timestamp();

    bool update(uint32_t tick) const;

    void markClean();
    void markDirty();

    static uint32_t now();

private:
    mutable uint32_t _last_modified;
};

}

#pragma once

#include "core/base/api.h"

namespace ark {

class ARK_API Timestamp {
public:
    Timestamp();

    bool update(uint64_t timestamp) const;

    void markClean();
    void markDirty();

    static uint64_t now();

private:
    mutable uint64_t _last_modified;
};

}

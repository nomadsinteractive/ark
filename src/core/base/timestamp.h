#ifndef ARK_CORE_BASE_TIMESTAMP_H_
#define ARK_CORE_BASE_TIMESTAMP_H_

#include "core/base/api.h"

namespace ark {

class ARK_API Timestamp {
public:
    Timestamp();

    bool update(uint64_t timestamp) const;

    void markDirty();

private:
    mutable uint64_t _last_modified;

};

}
#endif

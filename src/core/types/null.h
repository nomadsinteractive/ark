#ifndef ARK_CORE_TYPES_NULL_H_
#define ARK_CORE_TYPES_NULL_H_

#include "core/base/api.h"

namespace ark {

class ARK_API Null {
public:
    template<typename T> static const sp<T>& ptr();

    template<typename T> static const sp<T>& toSafe(const sp<T>& p) {
        return p ? p : ptr<T>();
    }
};

}

#endif

#ifndef ARK_CORE_TYPES_NULL_H_
#define ARK_CORE_TYPES_NULL_H_

#include "core/base/api.h"

namespace ark {

class ARK_API Null {
public:
    template<typename T> static sp<T> toSafe(const sp<T>& p) {
        return toSafe_sfinae(p, nullptr);
    }

private:
    template<typename T> static sp<T> ptr();

    template<typename T> static sp<T> toSafe_sfinae(const sp<T>& p, typename T::Null* /*arg*/) {
        static sp<T> safeNull = sp<typename T::Null>::make();
        return p ? p : safeNull;
    }

    template<typename T> static sp<T> toSafe_sfinae(const sp<T>& p, ...) {
        return p ? p : ptr<T>();
    }
};

}

#endif

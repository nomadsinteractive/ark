#pragma once

#include "core/base/api.h"

namespace ark {

class ARK_API Null {
public:
    template<typename T> static sp<T> toSafePtr(const sp<T>& p) {
        return toSafePtr_sfinae(p, nullptr);
    }

    template<typename T> static sp<T> safePtr();

private:
    template<typename T> static sp<T> toSafePtr_sfinae(const sp<T>& p, typename T::Null* /*arg*/) {
        static sp<T> safeNull = sp<typename T::Null>::make();
        return p ? p : safeNull;
    }

    template<typename T> static sp<T> toSafePtr_sfinae(const sp<T>& p, ...) {
        return p ? p : safePtr<T>();
    }
};

}

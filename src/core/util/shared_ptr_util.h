#ifndef ARK_CORE_UTIL_SHARED_PTR_UTIL_H_
#define ARK_CORE_UTIL_SHARED_PTR_UTIL_H_

#include "core/base/delegate.h"
#include "core/types/shared_ptr.h"

namespace ark {

class SharedPtrUtil {
public:
    template<typename T, typename U> static sp<T> as(const sp<U>& ptr) {
        const sp<T> directConvertedPtr = ptr.template as<T>();
        if(directConvertedPtr)
            return directConvertedPtr;

        const auto delegatePtr = ptr.template as<Delegate<T>>();
        if(delegatePtr)
            return delegatePtr->delegate();

        return nullptr;
    }
};

}

#endif

#ifndef ARK_CORE_UTIL_HOLDER_UTIL_H_
#define ARK_CORE_UTIL_HOLDER_UTIL_H_

#include "core/base/delegate.h"
#include "core/inf/holder.h"
#include "core/types/shared_ptr.h"

namespace ark {

class HolderUtil {
public:

    template<typename T> static int32_t visit(const sp<T>& inst, const Holder::Visitor& visitor) {
        int32_t r = HolderUtil::traverse(inst, visitor);
        if(r)
            return r;
        return visitor(inst.pack());
    }

    template<typename T> static int32_t traverse(const sp<T>& inst, const Holder::Visitor& visitor) {
        const sp<Holder> holder = inst.template as<Holder>();
        int32_t traversed = holder ? holder->traverse(visitor) : 0;
        if(!traversed) {
            const sp<Delegate<T>> delegate = inst.template as<Delegate<T>>();
            if(delegate)
                return traverse(delegate->delegate(), visitor);
        }
        return traversed;
    }

};

}

#endif

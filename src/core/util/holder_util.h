#ifndef ARK_CORE_UTIL_HOLDER_UTIL_H_
#define ARK_CORE_UTIL_HOLDER_UTIL_H_

#include "core/base/delegate.h"
#include "core/inf/holder.h"
#include "core/inf/flatable.h"
#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/types/box.h"
#include "core/types/shared_ptr.h"

namespace ark {

class HolderUtil {
public:
    template<typename T> static void visit(const SafePtr<T>& inst, const Holder::Visitor& visitor) {
        visit<T>(static_cast<sp<T>>(inst), visitor);
    }

    template<typename T> static void visit(const sp<T>& inst, const Holder::Visitor& visitor) {
        if(inst) {
            Box packed = inst;
            if(visitor(packed))
                return;
            traverse(inst, visitor);
        }
    }

private:
    template<typename T> static void traverse(const sp<T>& inst, const Holder::Visitor& visitor) {
        if(inst) {
            const sp<Holder> holder = inst.template as<Holder>();
            if(holder) {
                holder->traverse(visitor);
                return;
            }
            traverse_delegate_sfinae(inst, visitor, nullptr);
        }
    }

    template<typename T> static void traverse_delegate_sfinae(const sp<T>& inst, const Holder::Visitor& visitor, typename std::remove_reference<typename std::remove_cv<decltype(inst->delegate())>::type>::type::_PtrType*) {
        return visit(inst->delegate(), visitor);
    }

    template<typename T> static void traverse_delegate_sfinae(const sp<T>& inst, const Holder::Visitor& visitor, ...) {
        if(std::is_abstract<T>::value) {
            const sp<Delegate<T>> delegate = inst.template as<Delegate<T>>();
            if(delegate)
                return visit(delegate->delegate(), visitor);
        }
    }

};

}

#endif

#ifndef ARK_CORE_UTIL_HOLDER_UTIL_H_
#define ARK_CORE_UTIL_HOLDER_UTIL_H_

#include "core/base/delegate.h"
#include "core/base/scope.h"
#include "core/base/string.h"
#include "core/inf/holder.h"
#include "core/types/box.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "app/forwarding.h"

namespace ark {

class HolderUtil {
public:
    template<typename T> static void visit(const SafePtr<T>& inst, const Holder::Visitor& visitor) {
        visit<T>(static_cast<sp<T>>(inst), visitor);
    }

    template<typename T> static void visit(const sp<T>& inst, const Holder::Visitor& visitor) {
        if(inst) {
            if(visitor(inst))
                return;
            traverse(inst, visitor);
        }
    }

    static void visit(const Scope& scope, const Holder::Visitor& visitor) {
        for(const auto& iter : scope.variables())
            visit_box<Boolean, Integer, Numeric, Renderer, Arena, Vec2, Vec3, Vec4>(iter.second, visitor);
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
        const sp<Delegate<T>> delegate = inst.template as<Delegate<T>>();
        if(delegate)
            return visit(delegate->delegate(), visitor);
    }

    template<typename T = void, typename... Args> static void visit_box(const Box& packed, const Holder::Visitor& visitor) {
        if(!std::is_same<T, void>::value) {
            if(Type<T>::id() == packed.typeId())
                doVisit(packed.unpack<T>(), visitor);
            visit_box<Args...>(packed, visitor);
        }
    }

    template<typename T> static void doVisit(const sp<T>& obj, const Holder::Visitor& visitor) {
        HolderUtil::visit<T>(obj, visitor);
    }

    static void doVisit(const sp<void>& /*obj*/, const Holder::Visitor& /*visitor*/) {
    }

};

}

#endif

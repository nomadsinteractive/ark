#pragma once

#include "core/base/wrapper.h"
#include "core/base/scope.h"
#include "core/base/string.h"
#include "core/inf/debris.h"
#include "core/types/box.h"
#include "core/types/safe_var.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "app/forwarding.h"

namespace ark {

class HolderUtil {
public:
    template<typename T> static void visit(const SafePtr<T>& inst, const Debris::Visitor& visitor) {
        visit<T>(static_cast<sp<T>>(inst), visitor);
    }

    template<typename T> static void visit(const sp<T>& inst, const Debris::Visitor& visitor) {
        if(inst) {
            if(visitor(inst))
                return;
            traverse(inst, visitor);
        }
    }

    static void visit(const Scope& scope, const Debris::Visitor& visitor) {
        for(const auto& iter : scope.variables())
            visit_box<Boolean, Integer, Numeric, Renderer, Arena, Vec2, Vec3, Vec4>(iter.second, visitor);
    }

private:
    template<typename T> static void traverse(const sp<T>& inst, const Debris::Visitor& visitor) {
        if(inst) {
            const sp<Debris> holder = inst.template tryCast<Debris>();
            if(holder) {
                holder->traverse(visitor);
                return;
            }
            traverse_delegate_sfinae(inst, visitor, nullptr);
        }
    }

    template<typename T> static void traverse_delegate_sfinae(const sp<T>& inst, const Debris::Visitor& visitor, typename std::remove_reference_t<std::remove_cv_t<decltype(inst->delegate())>>::_PtrType*) {
        return visit(inst->delegate(), visitor);
    }

    template<typename T> static void traverse_delegate_sfinae(const sp<T>& inst, const Debris::Visitor& visitor, ...) {
        const sp<Wrapper<T>> delegate = inst.template tryCast<Wrapper<T>>();
        if(delegate)
            return visit(delegate->wrapped(), visitor);
    }

    template<typename T, typename... Args> static void visit_box(const Box& packed, const Debris::Visitor& visitor) {
        if(Type<T>::id() == packed.typeId())
            doVisit(packed.toPtr<T>(), visitor);
        if constexpr(sizeof...(Args) != 0)
            visit_box<Args...>(packed, visitor);
    }

    template<typename T> static void doVisit(const sp<T>& obj, const Debris::Visitor& visitor) {
        HolderUtil::visit<T>(obj, visitor);
    }

    static void doVisit(const sp<void>& /*obj*/, const Debris::Visitor& /*visitor*/) {
    }

};

}

#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"

namespace ark {

//[[script::bindings::class("Ref")]]
class ARK_API RefType {
public:
//  [[script::bindings::operator(index)]]
//  [[script::bindings::property]]
    static RefId id(const sp<Ref>& self);

    static bool isDiscarded(const sp<Ref>& self);
//  [[script::bindings::property]]
    static const SafeVar<Boolean>& discarded(const sp<Ref>& self);
//  [[script::bindings::property]]
    static void setDiscarded(const sp<Ref>& self, sp<Boolean> discarded);

//  [[script::bindings::auto]]
    static void discard(const sp<Ref>& self);
};

}
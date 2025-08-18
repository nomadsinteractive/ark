#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"

#include "app/forwarding.h"

namespace ark {

//[[script::bindings::class("Ref")]]
class ARK_API RefType {
public:
//  [[script::bindings::constructor]]
    static sp<Ref> create(RefId refid);

//  [[script::bindings::operator(index)]]
//  [[script::bindings::property]]
    static RefId id(const sp<Ref>& self);

//  [[script::bindings::operator(bool)]]
    static bool toBool(const sp<Ref>& self);

//  [[script::bindings::property]]
    static const OptionalVar<Boolean>& discarded(const sp<Ref>& self);
//  [[script::bindings::property]]
    static void setDiscarded(const sp<Ref>& self, sp<Boolean> discarded);

//  [[script::bindings::classmethod]]
    static void discard(const sp<Ref>& self);

//  [[script::bindings::classmethod]]
    static sp<Entity> toEntity(const sp<Ref>& self);
};

}

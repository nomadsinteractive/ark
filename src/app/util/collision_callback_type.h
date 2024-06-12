#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "app/forwarding.h"

namespace ark {

//[[script::bindings::class("CollisionCallback")]]
class ARK_API CollisionCallbackType final {
public:
//  [[script::bindings::constructor]]
    static sp<CollisionCallback> create(sp<CollisionCallback> value);
//  [[script::bindings::constructor]]
    static sp<CollisionCallback> create(String value);

};

}

#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"

#include "app/forwarding.h"

namespace ark {

//[[script::bindings::class("CollisionCallback")]]
class ARK_API CollisionCallbackType final {
public:
//  [[script::bindings::constructor]]
    static sp<CollisionCallback> create(Behavior& behavior);
    static sp<CollisionCallback> create(Behavior& behavior, StringView onBeginContact, StringView onEndContact);

};

}

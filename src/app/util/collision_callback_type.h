#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "app/forwarding.h"

namespace ark {

//[[script::bindings::class("CollisionCallback")]]
class ARK_API CollisionCallbackType final {
public:
//  [[script::bindings::constructor]]
    static sp<CollisionCallback> create(Behavior& behavior, StringView onBeginContact = "on_begin_contact", StringView onEndContact = "on_end_contact");

};

}

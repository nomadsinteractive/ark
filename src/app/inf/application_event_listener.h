#pragma once

#include "core/base/api.h"
#include "app/forwarding.h"

namespace ark {

class ARK_API ApplicationEventListener {
public:
    virtual ~ApplicationEventListener() = default;

//  [[script::bindings::interface]]
    virtual void onPause() = 0;
//  [[script::bindings::interface]]
    virtual void onResume() = 0;
//  [[script::bindings::interface]]
    virtual bool onUnhandledEvent(const Event& event) = 0;
};

}

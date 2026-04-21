#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[script::bindings::class("Runnable")]]
class ARK_API RunnableType final {
public:

//  [[script::bindings::operator(call)]]
    static void run(const sp<Runnable>& self);

//  [[script::bindings::classmethod]]
    static sp<Runnable> defer(sp<Runnable> self, float delay = 0.0f, sp<Boolean> canceled = nullptr);
};

}

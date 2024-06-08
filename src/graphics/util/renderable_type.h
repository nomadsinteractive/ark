#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API RenderableType {
public:
//  [[script::bindings::constructor]]
    static sp<Renderable> create(sp<Renderable> renderable, sp<Updatable> updatable = nullptr, sp<Boolean> discarded = nullptr);
};

}

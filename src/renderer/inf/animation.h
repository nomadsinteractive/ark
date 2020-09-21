#ifndef ARK_RENDERER_INF_ANIMATION_H_
#define ARK_RENDERER_INF_ANIMATION_H_

#include "core/base/api.h"
#include "core/forwarding.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Animation {
public:
    virtual ~Animation() = default;

//  [[script::bindings::auto]]
    virtual sp<Input> makeTransforms(const sp<Numeric>& duration) = 0;

//  [[script::bindings::property]]
    virtual float duration() = 0;

};

}

#endif

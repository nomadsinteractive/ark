#ifndef ARK_RENDERER_INF_ANIMATION_H_
#define ARK_RENDERER_INF_ANIMATION_H_

#include <vector>

#include "core/base/api.h"
#include "core/forwarding.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Animation {
public:
    virtual ~Animation() = default;

    virtual float duration() = 0;
    virtual sp<AnimationInput> makeTransforms(sp<Numeric> duration) = 0;
    virtual const std::vector<String>& nodeNames() = 0;
};

}

#endif

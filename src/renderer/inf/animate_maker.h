#ifndef ARK_RENDERER_INF_ANIMATE_MAKER_H_
#define ARK_RENDERER_INF_ANIMATE_MAKER_H_

#include "core/base/api.h"
#include "core/forwarding.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API AnimateMaker {
public:
    virtual ~AnimateMaker() = default;

    virtual sp<Animate> makeAnimate(const sp<Numeric>& duration) = 0;

};

}

#endif

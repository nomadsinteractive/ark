#ifndef ARK_RENDERER_INF_ANIMATION_INPUT_H_
#define ARK_RENDERER_INF_ANIMATION_INPUT_H_

#include "core/base/api.h"
#include "core/inf/input.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API AnimationInput : public Input {
public:
    virtual ~AnimationInput() = default;

//  [[script::bindings::auto]]
    virtual sp<Mat4> getNodeMatrix(const String& name) = 0;

};

}

#endif

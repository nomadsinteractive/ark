#ifndef ARK_GRAPHICS_IMPL_VEC_VEC2_WITH_TRANSFORM_H_
#define ARK_GRAPHICS_IMPL_VEC_VEC2_WITH_TRANSFORM_H_

#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class Vec2WithTransform : public Vec2 {
public:
    Vec2WithTransform(const sp<Vec2>& delegate, const sp<Transform>& transform);

    virtual V2 val() override;

private:
    sp<Vec2> _delegate;
    sp<Transform> _transform;
};

}

#endif

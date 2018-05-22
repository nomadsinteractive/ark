#ifndef ARK_GRAPHICS_IMPL_VV2_VV2_WITH_TRANSFORM_H_
#define ARK_GRAPHICS_IMPL_VV2_VV2_WITH_TRANSFORM_H_

#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class Vec2WithTransform : public Vec2 {
public:
    Vec2WithTransform(const sp<Vec2>& delegate, const sp<Vec2>& org, const sp<Transform>& transform);

    virtual V2 val() override;

private:
    const sp<Vec2> _delegate;
    const sp<Vec2> _org;
    const sp<Transform> _transform;
};

}

#endif
